#include "core/CommandTypes/SET/SET_RESPONSE_VERIFIER.h"

auto SET_RESPONSE_VERIFIER::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    if (data.size() < 3)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Frame muito curto para conter o cabeçalho mínimo de um SET-Response (Mínimo de 3 bytes).";
        response.errors.push_back(err);
        return response;
    }
    response.valid = true;

    const auto subType = static_cast<ServiceType>(data[1]);
    uint8_t invokeId = data[2];

    uint8_t resultChoice = 0;
    uint32_t blockNum = 0;
    uint8_t listCount = 0;
    size_t currentOffset = 0;

    ParsedField choiceField;
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
    case ServiceType::NORMAL:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "SET-Response-Normal inválido. Falta o campo de resultado (ActionResult).";
            response.errors.push_back(err);
            break;
        }

        resultChoice = data[3];
        choiceField.name = "Result (ActionResult)";
        choiceField.offset = 3;
        choiceField.length = 1;

        if (resultChoice == 0x00)
        {
            choiceField.value = "0x00";
            choiceField.description = "Success - Operação executada com sucesso.";
            response.fields.push_back(choiceField);
        }
        else
        {
            std::ostringstream errSs;
            errSs << "0x" << std::hex << std::uppercase << (int)resultChoice;
            choiceField.value = errSs.str();
            choiceField.description = "Matching Error / Falha na execução do SET.";
            response.fields.push_back(choiceField);
        }

        if (data.size() > 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 4;
            err.message = "Bytes sobressalentes redundantes detectados no final do SET-Response-Normal.";
            response.errors.push_back(err);
        }
        break;

    case ServiceType::WITH_DATABLOCK:
        if (data.size() < 7)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "SET-Response-with-DataBlock incompleto. Faltam bytes do Block Number.";
            response.errors.push_back(err);
            break;
        }

        blockNum = ((uint32_t)data[3] << 24) | ((uint32_t)data[4] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[6];

        blockField.name = "Block Number";
        blockField.offset = 3;
        blockField.length = 4;
        blockField.value = std::to_string(blockNum);
        blockField.description = "Número sequencial do bloco confirmado pelo servidor";
        response.fields.push_back(blockField);

        if (data.size() > 7)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 7;
            err.message = "Bytes sobressalentes redundantes detectados no final do SET-Response-with-DataBlock.";
            response.errors.push_back(err);
        }
        break;

    case ServiceType::WITH_LIST:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "SET-Response-with-List inválido. Falta a contagem de elementos da lista.";
            response.errors.push_back(err);
            break;
        }

        listCount = data[3];

        countField.name = "Result List Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de resultados retornados na lista";
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

            uint8_t itemResult = data[currentOffset];
            currentOffset += 1;
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
        err.message = "Subtipo de formato de resposta SET desconhecido.";
        response.errors.push_back(err);
        break;
    }

    return response;
}
