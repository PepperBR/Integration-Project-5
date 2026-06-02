#include "core/CommandTypes/GET/GET_REQUEST_VERIFIER.h"

auto GET_REQUEST_VERIFIER::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    if (data.size() < 3)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Frame muito curto para conter o cabeçalho mínimo de um GET-Request (Mínimo de 3 bytes).";
        response.errors.push_back(err);
        return response;
    }
    response.valid = true;

    const auto subType = static_cast<GetServiceType>(data[1]);
    uint8_t invokeId = data[2];

    constexpr size_t expectedMinimumSize = 12;
    constexpr size_t expectedSize = 7;
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
    case GetServiceType::NORMAL:

        if (data.size() < expectedMinimumSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "GET-Request-Normal incompleto. Faltam bytes do COSEM Attribute Descriptor (Cosem-Class, OBIS, ou Attribute-ID).";
            response.errors.push_back(err);
            break;
        }

        descriptorField.name = "COSEM Attribute Descriptor";
        descriptorField.offset = 3;
        descriptorField.length = 9;
        descriptorField.description = "Identificador do Objeto e Atributo solicitado";
        response.fields.push_back(descriptorField);

        if (data.size() > expectedMinimumSize)
        {
            ParsedField accessField;
            accessField.name = "Access Selection Parameters";
            accessField.offset = expectedMinimumSize;
            accessField.length = data.size() - expectedMinimumSize;
            accessField.description = "Parâmetros de restrição/filtro de acesso (Seletivo)";
            response.fields.push_back(accessField);
        }
        break;

    case GetServiceType::WITH_DATABLOCK:

        if (data.size() != expectedSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "GET-Request-with-DataBlock inválido. Deve conter exatamente 7 bytes (Header + 4 bytes de Block Number).";
            response.errors.push_back(err);
            break;
        }

        blockField.name = "Block Number";
        blockField.offset = 3;
        blockField.length = 4;

        blockNum = ((uint32_t)data[3] << 24) | ((uint32_t)data[4] << 16) | ((uint32_t)data[5] << 8) | (uint32_t)data[6];
        blockField.value = std::to_string(blockNum);
        blockField.description = "Número sequencial do bloco de dados solicitado";
        response.fields.push_back(blockField);
        break;

    case GetServiceType::WITH_LIST:
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "GET-Request-with-List inválido. Falta a especificação de quantidade da lista.";
            response.errors.push_back(err);
            break;
        }

        listCount = data[3];

        countField.name = "Result List Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de atributos solicitados na mesma lista";
        response.fields.push_back(countField);

        expectedListSize = 4 + (listCount * 9);

        if (data.size() < expectedListSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 4;
            err.message = "GET-Request-with-List incompleto. O número de bytes não atende a quantidade de objetos declarada na lista.";
            response.errors.push_back(err);
        }
        else if (data.size() > expectedListSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = expectedListSize;
            err.message = "Bytes adicionais/sobressalentes detectados no fim da estrutura da lista.";
            response.errors.push_back(err);
        }
        break;

    default:
        response.valid = false;
        ValidationError err;
        err.offset = 1;
        err.message = "Subtipo de formato de requisição desconhecido.";
        response.errors.push_back(err);
        break;
    }

    return response;
}