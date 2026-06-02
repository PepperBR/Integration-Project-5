#include "core/CommandTypes/ACTIONS/ACTIONS_RESPONSE_VERIFIER.h"

auto ACTIONS_RESPONSE_VERIFIER::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    if (data.size() < 3)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Frame muito curto para conter o cabeçalho mínimo de um ACTION-Response (Mínimo de 3 bytes).";
        response.errors.push_back(err);
        return response;
    }
    response.valid = true;

    const auto subType = static_cast<ServiceType>(data[1]);
    uint8_t invokeId = data[2];

    constexpr size_t expectedBlockHeaderSize = 10;
    uint8_t actionResult = 0;
    uint8_t lastBlock = 0;
    uint32_t blockNum = 0;
    uint16_t rawLength = 0;
    uint8_t listCount = 0;
    size_t currentOffset = 0;

    ParsedField resultField;
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
    case ServiceType::NORMAL:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "ACTION-Response-Normal inválido. Falta o campo de resultado obrigatório (ActionResult/Result).";
            response.errors.push_back(err);
            break;
        }

        actionResult = data[3];
        resultField.name = "ActionResult (Result)";
        resultField.offset = 3;
        resultField.length = 1;

        if (actionResult == 0x00)
        {
            resultField.value = "0x00";
            resultField.description = "Success - Ação executada com sucesso.";
            response.fields.push_back(resultField);

            if (data.size() > 4)
            {
                ParsedField returnParamsField;
                returnParamsField.name = "Response Parameters Optional Data";
                returnParamsField.offset = 4;
                returnParamsField.length = data.size() - 4;
                returnParamsField.description = "Dados ou parâmetros de retorno opcionais resultantes da execução da ação";
                response.fields.push_back(returnParamsField);
            }
        }
        else
        {
            std::ostringstream errSs;
            errSs << "0x" << std::hex << std::uppercase << (int)actionResult;
            resultField.value = errSs.str();
            resultField.description = "Falha / Erro na execução da ação (Data_Access_Result ou equivalente).";
            response.fields.push_back(resultField);

            if (data.size() > 4)
            {
                response.valid = false;
                ValidationError err;
                err.offset = 4;
                err.message = "Bytes adicionais redundantes encontrados após um ActionResult de falha.";
                response.errors.push_back(err);
            }
        }
        break;

    case ServiceType::WITH_DATABLOCK:
        if (data.size() < expectedBlockHeaderSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "ACTION-Response-with-DataBlock incompleto. Faltam parâmetros estruturais do bloco de resposta.";
            response.errors.push_back(err);
            break;
        }

        lastBlock = data[3];
        blockNum = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
        rawLength = ((uint16_t)data[8] << 8) | (uint16_t)data[9];

        lastBlockField.name = "Last Block Flag";
        lastBlockField.offset = 3;
        lastBlockField.length = 1;
        lastBlockField.value = (lastBlock == 0x01) ? "true" : "false";
        lastBlockField.description = (lastBlock == 0x01) ? "Último bloco de dados da resposta." : "Existem mais blocos de resposta pendentes.";
        response.fields.push_back(lastBlockField);

        blockField.name = "Block Number";
        blockField.offset = 4;
        blockField.length = 4;
        blockField.value = std::to_string(blockNum);
        blockField.description = "Índice sequencial do bloco de dados retornado";
        response.fields.push_back(blockField);

        if (data.size() != (expectedBlockHeaderSize + rawLength))
        {
            response.valid = false;
            ValidationError err;
            err.offset = 8;
            err.message = "Tamanho real do frame não condiz com o comprimento de dados brutos (raw-data length) declarado no DataBlock.";
            response.errors.push_back(err);
        }
        break;

    case ServiceType::WITH_LIST:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "ACTION-Response-with-List inválido. Falta a contagem de elementos da lista.";
            response.errors.push_back(err);
            break;
        }

        listCount = data[3];

        countField.name = "Action List Response Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de resultados retornados na lista de ações executadas";
        response.fields.push_back(countField);

        currentOffset = 4;
        for (int i = 0; i < listCount; ++i)
        {
            if (currentOffset >= data.size())
            {
                response.valid = false;
                ValidationError err;
                err.offset = currentOffset;
                err.message = "Estrutura de lista de respostas truncada. Faltam elementos para preencher a contagem declarada.";
                response.errors.push_back(err);
                return response;
            }

            uint8_t itemResultChoice = data[currentOffset];
            currentOffset += 1;

            if (itemResultChoice == 0x00)
            {
                if (currentOffset < data.size())
                {
                    currentOffset += 1;
                }
            }
            else if (itemResultChoice == 0x01)
            {
                if (currentOffset >= data.size())
                {
                    response.valid = false;
                    ValidationError err;
                    err.offset = currentOffset;
                    err.message = "Item da lista declarou erro, mas o byte com a causa (Data-Access-Result) está ausente.";
                    response.errors.push_back(err);
                    return response;
                }
                currentOffset += 1;
            }
        }

        if (response.valid && currentOffset < data.size())
        {
            response.valid = false;
            ValidationError err;
            err.offset = currentOffset;
            err.message = "Bytes adicionais redundantes encontrados após a deserialização de todos os itens da lista de ações.";
            response.errors.push_back(err);
        }
        break;

    default:
        response.valid = false;
        ValidationError err;
        err.offset = 1;
        err.message = "Subtipo de formato de resposta ACTION desconhecido.";
        response.errors.push_back(err);
        break;
    }

    return response;
}