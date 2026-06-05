#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"

#include "core/enums.h"
#include <variant>

auto GetRequestParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    auto serviceType = static_cast<ServiceType>(data[1]);
    auto invokeInfo = ParseHeader::decodeInvokeField(data[2]);

    auto headerResult = ParseHeader::parse_header(serviceType, invokeInfo.priority, invokeInfo.serviceClass);

    if (std::holds_alternative<ValidationError>(headerResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(headerResult));
        return response;
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(headerResult));
    }

    constexpr size_t minimumSize = 12;

    if (data.size() < minimumSize)
    {
        response.valid = false;

        response.errors.push_back({1, "GET-REQUEST-NORMAL incompleto.", ""});

        return response;
    }

    auto descriptorResult = Cosem_Attribute_Descriptor({data.begin() + 3, data.end()});

    if (std::holds_alternative<ParsedField>(descriptorResult))
    {
        response.fields.push_back(std::get<ParsedField>(descriptorResult));
    }
    else
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(descriptorResult));
    }
    return response;
}

auto GetRequestParser::Cosem_Attribute_Descriptor(const std::vector<uint8_t> &data) -> std::variant<ParsedField, ValidationError>
{
    if (data.size() < 9)
    {
        return ValidationError{1, "Dados insuficientes para um Descriptor de Atributo COSEM.", ""};
    }

    uint16_t classId = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    std::string obis = std::to_string(data[2]) + "." + std::to_string(data[3]) + "." + std::to_string(data[4]) + "." + std::to_string(data[5]) + "." +
                       std::to_string(data[6]) + "." + std::to_string(data[7]);
    uint8_t attributeId = data[8];

    return ParsedField{"Cosem-Attribute-Descriptor", 0, 9,
                       "ClassId: " + std::to_string(classId) + ", OBIS: " + obis + ", AttributeId: " + std::to_string(attributeId),
                       "Descritor de Atributo COSEM contendo ClassId, OBIS e AttributeId"};
}