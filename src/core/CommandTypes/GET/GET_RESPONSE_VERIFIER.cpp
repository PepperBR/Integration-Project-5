#include "core/CommandTypes/GET/GET_RESPONSE_VERIFIER.h"
#include <iomanip>
#include <sstream>

auto GET_RESPONSE_VERIFIER::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    if (data.size() < 3)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Frame muito curto para conter o cabeçalho mínimo de um GET-Response (Mínimo de 3 bytes).";
        response.errors.push_back(err);
        return response;
    }
    response.valid = true;

    const auto subType = static_cast<GetServiceType>(data[1]);
    uint8_t invokeId = data[2];

    constexpr size_t expectedHeaderSize = 11;
    uint8_t choice = 0;
    uint8_t lastBlock = 0;
    uint32_t blockNum = 0;
    uint16_t rawLength = 0;
    uint8_t listCount = 0;
    size_t currentOffset = 0;

    ParsedField choiceField;
    ParsedField lastBlockField;
    ParsedField invokeField;
    ParsedField blockField;
    ParsedField countField;

    invokeField.name = "Invoke-ID & Priority";
    invokeField.offset = 2;
    invokeField.length = 1;

    std::ostringstream invokeSs;
    invokeSs << "0x" << std::hex << std::uppercase << (int)invokeId;
    invokeField.value = invokeSs.str();
    invokeField.description = (invokeId & 0x80) ? "Alta Prioridade" : "Prioridade Normal";
    response.fields.push_back(invokeField);

    switch (subType)
    {
    case GetServiceType::NORMAL:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "GET-Response-Normal inválido. Falta o byte de escolha do resultado (GetDataResult).";
            response.errors.push_back(err);
            break;
        }

        choice = data[3];
        choiceField.name = "Result Choice (GetDataResult)";
        choiceField.offset = 3;
        choiceField.length = 1;

        if (choice == 0x00)
        {
            choiceField.value = "0x00";
            choiceField.description = "Sucesso - Conteúdo de dados presente.";
            response.fields.push_back(choiceField);

            if (data.size() < 5)
            {
                response.valid = false;
                ValidationError err;
                err.offset = 4;
                err.message = "GET-Response-Normal declarou sucesso mas não contém dados de payload.";
                response.errors.push_back(err);
            }
        }
        else if (choice == 0x01)
        {
            choiceField.value = "0x01";
            choiceField.description = "Erro - Falha no acesso aos dados.";
            response.fields.push_back(choiceField);

            if (data.size() != 5)
            {
                response.valid = false;
                ValidationError err;
                err.offset = 4;
                err.message =
                    "GET-Response-Normal com erro deve conter exatamente 5 bytes no total (Header + Choice + 1 byte de Data-Access-Result).";
                response.errors.push_back(err);
            }
        }
        else
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "Escolha inválida para GetDataResult (Deve ser 0x00 ou 0x01).";
            response.errors.push_back(err);
        }
        break;

    case GetServiceType::WITH_DATABLOCK:
        if (data.size() < expectedHeaderSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "GET-Response-with-DataBlock incompleto. Faltam parâmetros de controle do bloco.";
            response.errors.push_back(err);
            break;
        }

        lastBlock = data[3];
        blockNum = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
        choice = data[8];
        rawLength = ((uint16_t)data[9] << 8) | (uint16_t)data[10];

        lastBlockField.name = "Last Block Flag";
        lastBlockField.offset = 3;
        lastBlockField.length = 1;
        lastBlockField.value = (lastBlock == 0x01) ? "true" : "false";
        lastBlockField.description = (lastBlock == 0x01) ? "Último bloco da transmissão." : "Existem mais blocos pendentes.";
        response.fields.push_back(lastBlockField);

        blockField.name = "Block Number";
        blockField.offset = 4;
        blockField.length = 4;
        blockField.value = std::to_string(blockNum);
        blockField.description = "Índice sequencial do bloco de dados atual";
        response.fields.push_back(blockField);

        if (choice == 0x00)
        {
            size_t absoluteExpectedSize = expectedHeaderSize + rawLength;

            if (data.size() != absoluteExpectedSize)
            {
                response.valid = false;
                ValidationError err;
                err.offset = 9;
                err.message = "Tamanho real do frame divergente do comprimento bruto (raw-data length) especificado.";
                response.errors.push_back(err);
            }
        }
        else if (choice == 0x01)
        {
            if (rawLength != 0)
            {
                response.valid = false;
                ValidationError err;
                err.offset = 9;
                err.message = "Bloco com falha (Choice 0x01) deve declarar comprimento de dados igual a zero.";
                response.errors.push_back(err);
            }
        }
        else
        {
            response.valid = false;
            ValidationError err;
            err.offset = 8;
            err.message = "Escolha de bloco inválida (Deve ser 0x00 ou 0x01).";
            response.errors.push_back(err);
        }
        break;

    case GetServiceType::WITH_LIST:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "GET-Response-with-List inválido. Falta a contagem de elementos da lista.";
            response.errors.push_back(err);
            break;
        }

        listCount = data[3];

        countField.name = "Result List Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de resultados retornados nesta resposta";
        response.fields.push_back(countField);

        currentOffset = 4;
        for (int i = 0; i < listCount; ++i)
        {
            if (currentOffset >= data.size())
            {
                response.valid = false;
                ValidationError err;
                err.offset = currentOffset;
                err.message = "Estrutura de lista truncada. Faltam elementos para atender o Result List Count especificado.";
                response.errors.push_back(err);
                break;
            }

            uint8_t itemChoice = data[currentOffset];
            currentOffset += 1;

            if (itemChoice == 0x00)
            {
                if (currentOffset >= data.size())
                {
                    response.valid = false;
                    ValidationError err;
                    err.offset = currentOffset;
                    err.message = "Elemento da lista declarou dados de sucesso, mas o frame acabou.";
                    response.errors.push_back(err);
                    break;
                }

                currentOffset += 1;
            }
            else if (itemChoice == 0x01)
            {
                if (currentOffset >= data.size())
                {
                    response.valid = false;
                    ValidationError err;
                    err.offset = currentOffset;
                    err.message = "Elemento da lista declarou erro de acesso, mas o byte correspondente de código de erro está ausente.";
                    response.errors.push_back(err);
                    break;
                }
                currentOffset += 1;
            }
            else
            {
                response.valid = false;
                ValidationError err;
                err.offset = currentOffset - 1;
                err.message = "Item da lista possui uma flag de escolha inválida (Deve ser 0x00 ou 0x01).";
                response.errors.push_back(err);
                break;
            }
        }

        if (response.valid && currentOffset < data.size())
        {
            response.valid = false;
            ValidationError err;
            err.offset = currentOffset;
            err.message = "Bytes adicionais redundantes encontrados após a deserialização completa do array da lista.";
            response.errors.push_back(err);
        }
        break;

    default:
        response.valid = false;
        ValidationError err;
        err.offset = 1;
        err.message = "Subtipo de formato de resposta desconhecido.";
        response.errors.push_back(err);
        break;
    }

    return response;
}