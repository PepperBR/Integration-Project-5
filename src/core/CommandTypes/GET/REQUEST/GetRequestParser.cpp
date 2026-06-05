#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"

#include "core/enums.h"
#include <string>
#include <variant>
auto GetRequestParser::buildHeader(const std::vector<uint8_t> &data, VerifyFrameResponse &response) -> bool
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

auto GetRequestParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 3)
    {
        response.valid = false;
        response.errors.push_back({0, "Frame GET-REQUEST muito curto.", ""});
        return response;
    }

    switch (data[1])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyNext(data);
    case 0x03:
        return verifyWithList(data);
    default:
        response.valid = false;
        response.errors.push_back({1, "Sub-tipo de GET-REQUEST desconhecido: " + std::to_string(data[1]), ""});
        return response;
    }
}

auto GetRequestParser::verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = 13;
    if (data.size() < minimumSize)
    {
        response.valid = false;
        response.errors.push_back({1, "GET-REQUEST-NORMAL incompleto.", ""});
        return response;
    }

    auto descriptorResult = Cosem_Attribute_Descriptor(data, 3);
    if (std::holds_alternative<ParsedField>(descriptorResult))
    {
        response.fields.push_back(std::get<ParsedField>(descriptorResult));
    }
    else
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(descriptorResult));
        return response;
    }

    size_t selectionOffset = 12;
    uint8_t hasSelection = data[selectionOffset];
    if (hasSelection == 0x01)
    {
        if (data.size() <= selectionOffset + 1)
        {
            response.valid = false;
            response.errors.push_back({2, "Erro estrutural DLMS: Flag de selecao ativado (0x01), mas dados de selecao ausentes.", ""});
            return response;
        }

        response.fields.push_back({"Access-Selection", static_cast<int>(selectionOffset), 1, "1", "Selective-Access-Descriptor presente"});
    }
    else if (hasSelection == 0x00)
    {
        if (data.size() > selectionOffset + 1)
        {
            response.valid = false;
            response.errors.push_back({2, "Erro estrutural DLMS: Bytes extras detectados em um frame sem selecao de acesso.", ""});
            return response;
        }

        response.fields.push_back(
            {"Access-Selection", static_cast<int>(selectionOffset), 1, "0", "Sem seleção de acesso (acesso ao atributo completo)"});
    }
    else
    {
        response.valid = false;
        response.errors.push_back({2, "Valor invalido para flag Access-Selection: " + std::to_string(hasSelection), ""});
        return response;
    }

    return response;
}

auto GetRequestParser::verifyNext(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = 7;
    if (data.size() < minimumSize)
    {
        response.valid = false;
        response.errors.push_back({1, "GET-REQUEST-NEXT incompleto.", ""});
        return response;
    }

    uint32_t blockNumber = (static_cast<uint32_t>(data[3]) << 24) | (static_cast<uint32_t>(data[4]) << 16) | (static_cast<uint32_t>(data[5]) << 8) |
                           static_cast<uint32_t>(data[6]);

    response.fields.push_back({"Block-Number", 3, 4, std::to_string(blockNumber), "Número do próximo bloco de dados solicitado"});

    return response;
}
auto GetRequestParser::verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "GET-REQUEST-WITH-LIST incompleto. Ausencia do contador de elementos.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Attribute-Descriptor-List Count", 3, 1, std::to_string(count), "Numero de descritores de atributos na lista"});

    size_t offset = 4;

    for (uint8_t i = 0; i < count; ++i)
    {
        auto descResult = Cosem_Attribute_Descriptor(data, offset);

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

    if (offset < data.size())
    {
        uint8_t accessSelection = data[offset];
        response.fields.push_back({"Access-Selection Indicator", static_cast<int>(offset), 1, std::to_string(accessSelection),
                                   accessSelection == 0x00 ? "Sem selecao de acesso" : "Selecao de acesso / Filtro presente"});
        offset++;

        if (accessSelection != 0x00 && offset < data.size())
        {
            size_t remainingLen = data.size() - offset;
            response.fields.push_back({"Access-Selection Parameter / Data", static_cast<int>(offset), static_cast<int>(remainingLen),
                                       std::to_string(remainingLen) + " bytes", "Dados/Estrutura do filtro de selecao de acesso"});
            offset = data.size();
        }
    }

    if (offset < data.size())
    {
        response.valid = false;
        response.errors.push_back({2, "Erro estrutural: Bytes extras detectados ao final do frame GET-REQUEST-WITH-LIST.", ""});
        return response;
    }

    return response;
}

auto GetRequestParser::Cosem_Attribute_Descriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
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
                       "Descritor de Atributo COSEM contendo ClassId, OBIS e AttributeId"};
}