#include "core/CommandTypes/GET/RESPONSE/GetResponseParser.h"
#include "core/enums.h"
#include <sstream>
#include <string>
#include <variant>

auto GetResponseParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 3)
    {
        response.valid = false;
        response.errors.push_back({0, "Frame GET-RESPONSE muito curto.", ""});
        return response;
    }

    auto serviceType = static_cast<ServiceType>(data[1]);
    auto invokeInfo = ParseHeader::decodeInvokeField(data[2]);

    auto headerResult = ParseHeader::parse_header(serviceType, invokeInfo.priority, invokeInfo.serviceClass);

    if (std::holds_alternative<ValidationError>(headerResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(headerResult));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(headerResult));

    switch (data[1])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyWithDatablock(data);
    case 0x03:
        return verifyWithList(data);
    default:
        response.valid = false;
        response.errors.push_back({1, "Sub-tipo de GET-RESPONSE desconhecido: " + std::to_string(data[1]), ""});
        return response;
    }
}

auto GetResponseParser::verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    constexpr size_t minimumSize = 4;
    if (data.size() < minimumSize)
    {
        response.valid = false;
        response.errors.push_back({1, "GET-RESPONSE-NORMAL incompleto.", ""});
        return response;
    }

    auto result = parseGetDataResult(data, 3);
    if (std::holds_alternative<ValidationError>(result))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(result));
        return response;
    }

    response.fields.push_back(std::get<ParsedField>(result));

    // CORREÇÃO: Validação de fim de frame baseada no consumo real calculado dinamicamente
    size_t consumed = static_cast<size_t>(std::get<ParsedField>(result).offset) + std::get<ParsedField>(result).length;
    if (consumed < data.size())
    {
        response.valid = false;
        response.errors.push_back({2, "Erro estrutural: Bytes extras detectados ao final do frame GET-RESPONSE-NORMAL.", ""});
    }

    return response;
}

auto GetResponseParser::verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "GET-RESPONSE-WITH-LIST incompleto. Ausencia do contador de elementos.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Response-List Count", 3, 1, std::to_string(count), "Numero de resultados na lista"});

    size_t offset = 4;

    for (uint8_t i = 0; i < count; ++i)
    {
        if (offset >= data.size())
        {
            response.valid = false;
            response.errors.push_back({1, "Lista de respostas truncada antes de processar o indice " + std::to_string(i), ""});
            return response;
        }

        auto result = parseGetDataResult(data, offset);
        if (std::holds_alternative<ValidationError>(result))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(result));
            return response;
        }

        auto field = std::get<ParsedField>(result);
        field.name = "Get-Data-Result[" + std::to_string(i) + "]";
        response.fields.push_back(field);

        offset += field.length;
    }

    if (offset < data.size())
    {
        response.valid = false;
        response.errors.push_back({2, "Erro estrutural: Bytes extras detectados ao final do frame GET-RESPONSE-WITH-LIST.", ""});
    }

    return response;
}

auto GetResponseParser::verifyWithDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    constexpr size_t minimumSize = 9;
    if (data.size() < minimumSize)
    {
        response.valid = false;
        response.errors.push_back({1, "GET-RESPONSE-WITH-DATABLOCK incompleto.", ""});
        return response;
    }

    auto result = parseDataBlockG(data, 3);
    if (std::holds_alternative<ValidationError>(result))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(result));
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(result));
    }

    return response;
}

auto GetResponseParser::parseDataAccessResult(uint8_t value, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    struct Entry
    {
        uint8_t code;
        const char *label;
    };
    static constexpr Entry table[] = {
        {0, "Success"},
        {1, "Hardware Fault"},
        {2, "Temporary Failure"},
        {3, "Read/Write Denied"},
        {4, "Object Undefined"},
        {9, "Object Class Inconsistent"},
        {11, "Object Unavailable"},
        {12, "Type Unmatched"},
        {13, "Scope of Access Violated"},
        {14, "Data Block Unavailable"},
        {15, "Long Get Aborted"},
        {16, "No Long Get In Progress"},
        {17, "Long Set Aborted"},
        {18, "No Long Set In Progress"},
        {19, "Data Block Number Invalid"},
        {250, "Other Reason"},
    };

    for (const auto &e : table)
    {
        if (e.code == value)
            return ParsedField{"Data-Access-Result", static_cast<int>(offset), 1, std::to_string(value), e.label};
    }
    return ValidationError{2, "Data-Access-Result desconhecido: " + std::to_string(value), ""};
}

static auto calculateCosemDataLength(const std::vector<uint8_t> &data, size_t &currentOffset) -> bool
{
    if (currentOffset >= data.size())
        return false;

    uint8_t tag = data[currentOffset++];

    if (tag == 0x01 || tag == 0x02)
    {
        if (currentOffset >= data.size())
            return false;
        uint8_t elementCount = data[currentOffset++];

        for (uint8_t i = 0; i < elementCount; ++i)
        {
            if (!calculateCosemDataLength(data, currentOffset))
                return false;
        }
        return true;
    }

    if (tag == 0x09 || tag == 0x0A)
    {
        if (currentOffset >= data.size())
            return false;
        uint8_t length = data[currentOffset++];
        currentOffset += length;
        return currentOffset <= data.size();
    }

    size_t fixedSize = 0;
    switch (tag)
    {
    case 0x03:
        fixedSize = 1;
        break;
    case 0x04:
        fixedSize = 1;
        break;
    case 0x05:
        fixedSize = 4;
        break;
    case 0x06:
        fixedSize = 4;
        break;
    case 0x0F:
        fixedSize = 1;
        break;
    case 0x10:
        fixedSize = 2;
        break;
    case 0x11:
        fixedSize = 1;
        break;
    case 0x12:
        fixedSize = 2;
        break;
    case 0x14:
        fixedSize = 8;
        break;
    case 0x15:
        fixedSize = 8;
        break;
    case 0x16:
        fixedSize = 1;
        break;
    default:
        fixedSize = 0;
        break;
    }

    currentOffset += fixedSize;
    return currentOffset <= data.size();
}

auto GetResponseParser::parseGetDataResult(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    if (offset >= data.size())
        return ValidationError{1, "Get-Data-Result ausente.", ""};

    uint8_t choiceTag = data[offset];

    if (choiceTag == 0x00)
    {
        size_t inspectOffset = offset + 1;

        // Executa o motor dinâmico de parsing DLMS
        if (!calculateCosemDataLength(data, inspectOffset))
        {
            return ValidationError{1, "Erro estrutural: Payload de dados complexos esta incompleto ou corrompido.", ""};
        }

        size_t totalFieldLen = inspectOffset - offset;
        size_t payloadLen = totalFieldLen - 1;

        return ParsedField{"Get-Data-Result", static_cast<int>(offset), static_cast<int>(totalFieldLen), "Data",
                           "Dados presentes (Estrutura DLMS dinamica com " + std::to_string(payloadLen) + " bytes consumidos)"};
    }
    else if (choiceTag == 0x01)
    {
        if (offset + 1 >= data.size())
            return ValidationError{1, "Data-Access-Result ausente após tag de choice.", ""};

        auto darResult = parseDataAccessResult(data[offset + 1], offset + 1);
        if (std::holds_alternative<ValidationError>(darResult))
            return darResult;

        auto field = std::get<ParsedField>(darResult);
        field.length = 2;
        field.offset = static_cast<int>(offset);
        return field;
    }

    return ValidationError{1, "Tag de Get-Data-Result inválida: " + std::to_string(choiceTag), ""};
}

auto GetResponseParser::parseDataBlockG(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    if (offset + 5 >= data.size())
        return ValidationError{1, "DataBlock-G incompleto.", ""};

    bool lastBlock = data[offset] != 0x00;

    uint32_t blockNumber = (static_cast<uint32_t>(data[offset + 1]) << 24) | (static_cast<uint32_t>(data[offset + 2]) << 16) |
                           (static_cast<uint32_t>(data[offset + 3]) << 8) | static_cast<uint32_t>(data[offset + 4]);

    uint8_t resultTag = data[offset + 5];

    std::ostringstream oss;
    oss << "LastBlock: " << (lastBlock ? "true" : "false") << ", BlockNumber: " << blockNumber;

    if (resultTag == 0x00)
    {
        size_t rawLen = data.size() - offset - 6;
        oss << ", Result: raw-data (" << rawLen << " bytes)";
    }
    else if (resultTag == 0x01)
    {
        if (offset + 6 >= data.size())
            return ValidationError{1, "Data-Access-Result ausente em DataBlock-G.", ""};

        auto dar = parseDataAccessResult(data[offset + 6], offset + 6);
        if (std::holds_alternative<ValidationError>(dar))
            return dar;

        oss << ", Result: " << std::get<ParsedField>(dar).value;
    }
    else
    {
        return ValidationError{1, "Tag de result em DataBlock-G inválida: " + std::to_string(resultTag), ""};
    }

    return ParsedField{"DataBlock-G", static_cast<int>(offset), static_cast<int>(data.size() - offset), oss.str(),
                       "Bloco de dados GET (last-block, block-number, result)"};
}