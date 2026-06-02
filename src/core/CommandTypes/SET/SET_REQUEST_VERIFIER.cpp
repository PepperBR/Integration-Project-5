#include "core/CommandTypes/SET/SET_REQUEST_VERIFIER.h"

auto SET_REQUEST_VERIFIER::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    if (data.size() < 3)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Frame muito curto para conter o cabeçalho mínimo de um GET/SET-Request (Mínimo de 3 bytes).";
        response.errors.push_back(err);
        return response;
    }
    response.valid = true;

    const auto subType = static_cast<ServiceType>(data[1]);
    uint8_t invokeId = data[2];

    constexpr size_t expectedMinimumHeaderSize = 12;
    uint32_t blockNum = 0;
    uint8_t listCount = 0;

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
    case ServiceType::NORMAL: {
        if (data.size() < expectedMinimumHeaderSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "SET-Request-Normal incompleto. Faltam bytes do COSEM Attribute Descriptor (Cosem-Class, OBIS, ou Attribute-ID).";
            response.errors.push_back(err);
            break;
        }

        ParsedField descriptorField;
        descriptorField.name = "COSEM Attribute Descriptor";
        descriptorField.offset = 3;
        descriptorField.length = 9;
        descriptorField.description = "Identificador do Objeto e Atributo a ser alterado";
        response.fields.push_back(descriptorField);

        if (data.size() == expectedMinimumHeaderSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = expectedMinimumHeaderSize;
            err.message = "SET-Request-Normal inválido. Falta o valor/conteúdo de dados (Data {Data}) a ser gravado.";
            response.errors.push_back(err);
        }
        else
        {
            ParsedField dataField;
            dataField.name = "Value Data";
            dataField.offset = expectedMinimumHeaderSize;
            dataField.length = data.size() - expectedMinimumHeaderSize;
            dataField.description = "Conteúdo do valor/objeto enviado para escrita";
            response.fields.push_back(dataField);
        }
        break;
    }

    case ServiceType::WITH_DATABLOCK: {
        constexpr size_t expectedControlSize = 8;

        if (data.size() < expectedControlSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "SET-Request-with-DataBlock incompleto. Faltam parâmetros de controle de bloco (Last Block ou Block Number).";
            response.errors.push_back(err);
            break;
        }

        uint8_t lastBlock = data[3];
        blockNum = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];

        ParsedField lastBlockField;
        lastBlockField.name = "Last Block Flag";
        lastBlockField.offset = 3;
        lastBlockField.length = 1;
        lastBlockField.value = (lastBlock == 0x01) ? "true" : "false";
        lastBlockField.description = (lastBlock == 0x01) ? "Último bloco do arquivo." : "Existem mais blocos pendentes.";
        response.fields.push_back(lastBlockField);

        ParsedField blockField;
        blockField.name = "Block Number";
        blockField.offset = 4;
        blockField.length = 4;
        blockField.value = std::to_string(blockNum);
        blockField.description = "Número sequencial do bloco de dados atual";
        response.fields.push_back(blockField);

        if (data.size() == expectedControlSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = expectedControlSize;
            err.message = "SET-Request-with-DataBlock vazio. O bloco deve conter um payload de dados bruto (Raw_Data).";
            response.errors.push_back(err);
        }
        else
        {
            ParsedField rawDataField;
            rawDataField.name = "Raw Data Payload";
            rawDataField.offset = expectedControlSize;
            rawDataField.length = data.size() - expectedControlSize;
            rawDataField.description = "Segmento bruto de dados serializados pertencentes a este bloco";
            response.fields.push_back(rawDataField);
        }
        break;
    }

    case ServiceType::WITH_LIST: {
        if (data.size() < 4)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 3;
            err.message = "SET-Request-with-List inválido. Falta a especificação da quantidade de itens na lista.";
            response.errors.push_back(err);
            break;
        }

        listCount = data[3];

        ParsedField countField;
        countField.name = "Attribute List Count";
        countField.offset = 3;
        countField.length = 1;
        countField.value = std::to_string((int)listCount);
        countField.description = "Quantidade de atributos na lista de modificação";
        response.fields.push_back(countField);

        size_t expectedDescriptorsSize = listCount * 9;
        size_t headerAndDescriptorsSize = 4 + expectedDescriptorsSize;

        if (data.size() < headerAndDescriptorsSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 4;
            err.message = "SET-Request-with-List incompleto. Bytes insuficientes para conter os descritores declarados.";
            response.errors.push_back(err);
            break;
        }

        ParsedField descriptorListField;
        descriptorListField.name = "COSEM Attribute Descriptor List";
        descriptorListField.offset = 4;
        descriptorListField.length = expectedDescriptorsSize;
        descriptorListField.description = "Array contendo os descritores dos objetos";
        response.fields.push_back(descriptorListField);

        if (data.size() == headerAndDescriptorsSize)
        {
            response.valid = false;
            ValidationError err;
            err.offset = headerAndDescriptorsSize;
            err.message = "SET-Request-with-List inválido. Falta a lista correspondente de valores (Data List) para escrita.";
            response.errors.push_back(err);
        }
        else
        {
            ParsedField dataListField;
            dataListField.name = "Value Data List";
            dataListField.offset = headerAndDescriptorsSize;
            dataListField.length = data.size() - headerAndDescriptorsSize;
            dataListField.description = "Valores correspondentes enviados para gravação consecutiva";
            response.fields.push_back(dataListField);
        }
        break;
    }

    default:
        response.valid = false;
        ValidationError err;
        err.offset = 1;
        err.message = "Subtipo de formato de requisição SET desconhecido.";
        response.errors.push_back(err);
        break;
    }

    return response;
}