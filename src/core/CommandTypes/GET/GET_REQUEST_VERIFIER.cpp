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

    const auto subType = static_cast<ServiceType>(data[1]);
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
    case ServiceType::NORMAL:

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

    case ServiceType::WITH_DATABLOCK:

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

    case ServiceType::WITH_LIST: {
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

        countField.name = "Attribute Descriptor Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de atributos solicitados";
        response.fields.push_back(countField);

        size_t currentOffset = 4;

        for (uint8_t i = 0; i < listCount; ++i)
        {
            // 9 bytes do COSEM Attribute Descriptor
            if (currentOffset + 9 > data.size())
            {
                response.valid = false;

                ValidationError err;
                err.offset = currentOffset;
                err.message = "GET-Request-with-List incompleto. "
                              "COSEM Attribute Descriptor ausente ou truncado.";
                response.errors.push_back(err);

                break;
            }

            ParsedField descriptor;
            descriptor.name = "COSEM Attribute Descriptor #" + std::to_string(i + 1);
            descriptor.offset = currentOffset;
            descriptor.length = 9;
            descriptor.description = "Identificador do Objeto e Atributo solicitado";

            response.fields.push_back(descriptor);

            currentOffset += 9;

            // Access Selection (obrigatório)
            if (currentOffset >= data.size())
            {
                response.valid = false;

                ValidationError err;
                err.offset = currentOffset;
                err.message = "GET-Request-with-List incompleto. "
                              "Byte Access Selection ausente.";
                response.errors.push_back(err);

                break;
            }

            uint8_t accessSelection = data[currentOffset];

            ParsedField accessField;
            accessField.name = "Access Selection #" + std::to_string(i + 1);
            accessField.offset = currentOffset;
            accessField.length = 1;
            accessField.value = std::to_string(accessSelection);
            accessField.description = accessSelection == 0 ? "Sem acesso seletivo" : "Com acesso seletivo";

            response.fields.push_back(accessField);

            currentOffset += 1;

            if (accessSelection != 0)
            {
                response.valid = false;

                ValidationError err;
                err.offset = currentOffset - 1;
                err.message = "Access Selection diferente de zero ainda não é suportado pelo verificador.";
                response.errors.push_back(err);

                break;
            }
        }

        if (response.valid && currentOffset < data.size())
        {
            response.valid = false;

            ValidationError err;
            err.offset = currentOffset;
            err.message = "Bytes adicionais/sobressalentes detectados no fim da estrutura da lista.";
            response.errors.push_back(err);
        }

        break;
    }

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