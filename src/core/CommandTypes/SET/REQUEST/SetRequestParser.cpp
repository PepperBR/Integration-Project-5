#include "core/CommandTypes/SET/REQUEST/SetRequestParser.h"
#include "core/enums.h"
#include <sstream>
#include <string>
#include <variant>

auto SetRequestParser::buildHeader(const std::vector<uint8_t> &data, VerifyFrameResponse &response) -> bool
{
    auto serviceType = static_cast<ServiceType>(data[1]);
    auto invokeInfo = ParseHeader::decodeInvokeField(data[2]);
    auto headerResult = ParseHeader::parse_header(serviceType, invokeInfo.priority, invokeInfo.serviceClass);

    if (std::holds_alternative<ValidationError>(headerResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(headerResult));
        return false;
    }
    response.fields.push_back(std::get<ParsedField>(headerResult));
    return true;
}

auto SetRequestParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 3)
    {
        response.valid = false;
        response.errors.push_back({0, "Frame SET-REQUEST muito curto.", ""});
        return response;
    }

    switch (data[1])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyWithFirstDatablock(data);
    case 0x03:
        return verifyWithDatablock(data);
    case 0x04:
        return verifyWithList(data);
    case 0x05:
        return verifyWithListAndFirstDatablock(data);
    default:
        response.valid = false;
        response.errors.push_back({1, "Sub-tipo de SET-REQUEST desconhecido: " + std::to_string(data[1]), ""});
        return response;
    }
}

auto SetRequestParser::parseCosemAttributeDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    constexpr size_t descriptorSize = 9;
    if (offset + descriptorSize > data.size())
        return ValidationError{1, "Dados insuficientes para Cosem-Attribute-Descriptor.", ""};

    uint16_t classId = (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];

    std::string obis = std::to_string(data[offset + 2]) + "." + std::to_string(data[offset + 3]) + "." + std::to_string(data[offset + 4]) + "." +
                       std::to_string(data[offset + 5]) + "." + std::to_string(data[offset + 6]) + "." + std::to_string(data[offset + 7]);

    int8_t attributeId = static_cast<int8_t>(data[offset + 8]);

    return ParsedField{"Cosem-Attribute-Descriptor", static_cast<int>(offset), static_cast<int>(descriptorSize),
                       "ClassId: " + std::to_string(classId) + ", OBIS: " + obis + ", AttributeId: " + std::to_string(attributeId),
                       "Descritor de Atributo COSEM (ClassId, OBIS, AttributeId)"};
}

auto SetRequestParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    if (offset + 6 > data.size())
        return ValidationError{1, "DataBlock-SA incompleto ou sem definicao de tamanho.", ""};

    bool lastBlock = data[offset] != 0x00;

    uint32_t blockNumber = (static_cast<uint32_t>(data[offset + 1]) << 24) | (static_cast<uint32_t>(data[offset + 2]) << 16) |
                           (static_cast<uint32_t>(data[offset + 3]) << 8) | static_cast<uint32_t>(data[offset + 4]);

    size_t lengthOffset = offset + 5;
    uint8_t declaredBlockLen = data[lengthOffset];

    size_t actualBlockLen = data.size() - (lengthOffset + 1);

    if (actualBlockLen != declaredBlockLen)
    {
        return ValidationError{2,
                               "Erro estrutural no DataBlock-SA: Tamanho declarado (" + std::to_string(declaredBlockLen) +
                                   " bytes) difere dos bytes reais recebidos (" + std::to_string(actualBlockLen) + " bytes).",
                               ""};
    }

    std::ostringstream oss;
    oss << "LastBlock: " << (lastBlock ? "true" : "false") << ", BlockNumber: " << blockNumber << ", RawData: " << actualBlockLen << " bytes";

    size_t totalBlockSize = data.size() - offset;
    return ParsedField{"DataBlock-SA", static_cast<int>(offset), static_cast<int>(totalBlockSize), oss.str(),
                       "Bloco de dados para SET/ACTION (last-block, block-number, raw-data)"};
}

auto SetRequestParser::verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 13)
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-NORMAL incompleto.", ""});
        return response;
    }

    auto descResult = parseCosemAttributeDescriptor(data, 3);
    if (std::holds_alternative<ValidationError>(descResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(descResult));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(descResult));

    size_t dataOffset = 12;
    uint8_t hasSelection = data[dataOffset];
    if (hasSelection == 0x01)
    {
        response.fields.push_back({"Access-Selection", static_cast<int>(dataOffset), 1, "1", "Selective-Access-Descriptor presente"});
        dataOffset++;

        if (dataOffset >= data.size())
        {
            response.valid = false;
            response.errors.push_back({1, "SET-REQUEST-NORMAL invalido: Selective-Access-Descriptor ausente.", ""});
            return response;
        }
    }
    else
    {
        response.fields.push_back({"Access-Selection", static_cast<int>(dataOffset), 1, "0", "Sem seleção de acesso"});
        dataOffset++;
    }

    if (dataOffset >= data.size())
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-NORMAL invalido: Valor a ser escrito (Data value) ausente.", ""});
        return response;
    }

    size_t valueLen = data.size() - dataOffset;
    response.fields.push_back({"Data (value)", static_cast<int>(dataOffset), static_cast<int>(valueLen), std::to_string(valueLen) + " bytes",
                               "Valor a ser escrito no atributo COSEM"});

    return response;
}

auto SetRequestParser::verifyWithFirstDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 19)
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-WITH-FIRST-DATABLOCK incompleto.", ""});
        return response;
    }

    auto descResult = parseCosemAttributeDescriptor(data, 3);
    if (std::holds_alternative<ValidationError>(descResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(descResult));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(descResult));

    size_t offset = 12;
    uint8_t hasSelection = data[offset++];
    response.fields.push_back({"Access-Selection", static_cast<int>(offset - 1), 1, std::to_string(hasSelection),
                               hasSelection ? "Selective-Access-Descriptor presente" : "Sem seleção de acesso"});

    auto blockResult = parseDataBlockSA(data, offset);
    if (std::holds_alternative<ValidationError>(blockResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(blockResult));
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(blockResult));
    }

    return response;
}

auto SetRequestParser::verifyWithDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 9)
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-WITH-DATABLOCK incompleto.", ""});
        return response;
    }

    auto blockResult = parseDataBlockSA(data, 3);
    if (std::holds_alternative<ValidationError>(blockResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(blockResult));
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(blockResult));
    }

    return response;
}

auto SetRequestParser::verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-WITH-LIST incompleto.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Attribute-Descriptor-List Count", 3, 1, std::to_string(count), "Número de descritores na lista"});

    size_t offset = 4;
    for (uint8_t i = 0; i < count; ++i)
    {
        auto descResult = parseCosemAttributeDescriptor(data, offset);
        if (std::holds_alternative<ValidationError>(descResult))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(descResult));
            return response;
        }
        auto field = std::get<ParsedField>(descResult);
        field.name = "Cosem-Attribute-Descriptor[" + std::to_string(i) + "]";
        response.fields.push_back(field);

        offset += 9;
    }

    if (offset >= data.size())
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-WITH-LIST invalido: Lista de valores (Value-List) ausente.", ""});
        return response;
    }

    size_t valLen = data.size() - offset;
    response.fields.push_back({"Value-List", static_cast<int>(offset), static_cast<int>(valLen), std::to_string(valLen) + " bytes",
                               "Lista de valores Data a serem escritos"});

    return response;
}

auto SetRequestParser::verifyWithListAndFirstDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "SET-REQUEST-WITH-LIST-AND-FIRST-DATABLOCK incompleto.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Attribute-Descriptor-List Count", 3, 1, std::to_string(count), "Número de descritores na lista"});

    size_t offset = 4;
    for (uint8_t i = 0; i < count; ++i)
    {
        auto descResult = parseCosemAttributeDescriptor(data, offset);
        if (std::holds_alternative<ValidationError>(descResult))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(descResult));
            return response;
        }
        auto field = std::get<ParsedField>(descResult);
        field.name = "Cosem-Attribute-Descriptor[" + std::to_string(i) + "]";
        response.fields.push_back(field);
        offset += 9;
    }

    auto blockResult = parseDataBlockSA(data, offset);
    if (std::holds_alternative<ValidationError>(blockResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(blockResult));
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(blockResult));
    }

    return response;
}