#include "core/CommandTypes/ACTIONS/ACTIONS_REQUEST_VERIFIER.h"

auto ACTIONS_REQUEST_VERIFIER::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    if (data.size() < 3)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Frame muito curto para conter o cabeçalho mínimo de um ACTION-Request (Mínimo de 3 bytes).";
        response.errors.push_back(err);
        return response;
    }
    response.valid = true;

    const auto subType = static_cast<ServiceType>(data[1]);
    uint8_t invokeId = data[2];

    constexpr size_t expectedNormalMinSize = 12;
    constexpr size_t expectedBlockSize = 7;
    uint32_t blockNum = 0;
    uint8_t listCount = 0;
    size_t expectedListSize = 0;

    ParsedField countField;
    ParsedField blockField;
    ParsedField descriptorField;
    ParsedField invokeField;

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
        if (data.size() < expectedNormalMinSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "ACTION-Request-Normal incompleto. Faltam bytes do COSEM Method Descriptor (Cosem-Class, OBIS, ou Method-ID).";
            response.errors.push_back(err);
            break;
        }

        descriptorField.name = "COSEM Method Descriptor";
        descriptorField.offset = 3;
        descriptorField.length = 9;
        descriptorField.description = "Identificador do Objeto e Método para execução da Ação";
        response.fields.push_back(descriptorField);

        if (data.size() > expectedNormalMinSize)
        {
            ParsedField paramsField;
            paramsField.name = "Method Invocation Parameters";
            paramsField.offset = expectedNormalMinSize;
            paramsField.length = data.size() - expectedNormalMinSize;
            paramsField.description = "Parâmetros/Argumentos opcionais passados para a execução do método";
            response.fields.push_back(paramsField);
        }
        break;

    case ServiceType::WITH_DATABLOCK:
        if (data.size() < expectedBlockSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "ACTION-Request-with-DataBlock inválido. Deve conter pelo menos o Block Number (Mínimo 7 bytes).";
            response.errors.push_back(err);
            break;
        }

        blockNum = ((uint32_t)data[3] << 24) | ((uint32_t)data[4] << 16) | ((uint32_t)data[5] << 8) | (uint32_t)data[6];

        blockField.name = "Block Number";
        blockField.offset = 3;
        blockField.length = 4;
        blockField.value = std::to_string(blockNum);
        blockField.description = "Número sequencial do bloco de dados enviado/solicitado";
        response.fields.push_back(blockField);

        if (data.size() > expectedBlockSize)
        {
            ParsedField rawDataField;
            rawDataField.name = "DataBlock Raw Data";
            rawDataField.offset = expectedBlockSize;
            rawDataField.length = data.size() - expectedBlockSize;
            rawDataField.description = "Conteúdo parcial/bruto pertencente ao bloco de dados da ação";
            response.fields.push_back(rawDataField);
        }
        break;

    case ServiceType::WITH_LIST:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "ACTION-Request-with-List inválido. Falta a especificação de quantidade da lista.";
            response.errors.push_back(err);
            break;
        }

        listCount = data[3];

        countField.name = "Method List Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de métodos solicitados na mesma lista de execução";
        response.fields.push_back(countField);

        expectedListSize = 4 + (listCount * 9);

        if (data.size() < expectedListSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 4;
            err.message = "ACTION-Request-with-List incompleto. O tamanho do frame não atende a quantidade de métodos declarada.";
            response.errors.push_back(err);
        }
        else if (data.size() > expectedListSize)
        {
            ParsedField inlineParamsField;
            inlineParamsField.name = "List Trailing Parameters / Data";
            inlineParamsField.offset = expectedListSize;
            inlineParamsField.length = data.size() - expectedListSize;
            inlineParamsField.description = "Dados de parâmetros complementares associados aos métodos da lista";
            response.fields.push_back(inlineParamsField);
        }
        break;

    default:
        response.valid = false;
        ValidationError err;
        err.offset = 1;
        err.message = "Subtipo de formato de requisição ACTION desconhecido.";
        response.errors.push_back(err);
        break;
    }

    return response;
}