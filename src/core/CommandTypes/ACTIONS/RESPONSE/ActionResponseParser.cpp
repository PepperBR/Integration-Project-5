#include "core/CommandTypes/ACTIONS/RESPONSE/ActionResponseParser.h"
#include "core/enums.h"
#include "core/utils/CosemDataParser.h"
#include "core/utils/DlmsFrameUtils.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <variant>

static constexpr size_t PAYLOAD_OFFSET = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
static constexpr size_t RESULT_OFFSET = PAYLOAD_OFFSET;
static constexpr size_t RETURN_PARAMS_OFFSET = PAYLOAD_OFFSET + 1;
static constexpr size_t BLOCK_NUMBER_SIZE = DlmsFrameUtils::DATABLOCK_BLOCK_NUMBER_SIZE;

auto ActionResponseParser::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    return DlmsFrameUtils::buildHeader(data, response);
}

auto ActionResponseParser::verify(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-response";
    response.fields.name = "Action-Response";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    constexpr size_t MIN_FRAME_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
    if (data.size() < MIN_FRAME_SIZE)
    {
        response.error.emplace(Error{"Frame ACTION-RESPONSE muito curto."});
        return response;
    }

    switch (data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyWithPblock(data);
    case 0x03:
        response.error.emplace(
            Error{"ACTION-RESPONSE-WITH-LIST não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    case 0x04:
        return verifyNextPblock(data);
    default:
        response.error.emplace(Error{"Sub-tipo de ACTION-RESPONSE desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    }
}

auto ActionResponseParser::parseActionResult(uint8_t value, size_t /*offset*/) -> std::variant<ParsedField, Error>
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
        {13, "Scope Of Access Violated"},
        {14, "Data Block Unavailable"},
        {15, "Long Action Aborted"},
        {16, "No Long Action In Progress"},
        {250, "Other Reason"},
    };

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(value);

    for (const auto &e : table)
    {
        if (e.code == value)
        {
            ParsedField field;
            field.identifier = "action-result";
            field.name = "Action-Result";
            field.value_bytes = oss.str();
            field.values.push_back({e.label, ""});
            return field;
        }
    }
    return Error{"Action-Result desconhecido: " + std::to_string(value)};
}

auto ActionResponseParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return DlmsFrameUtils::parseDataBlockSA(data, offset);
}

auto ActionResponseParser::verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-response-normal";
    response.fields.name = "Action-Response-Normal";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = RETURN_PARAMS_OFFSET;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-RESPONSE-NORMAL incompleto."});
        return response;
    }

    auto resultField = parseActionResult(data[RESULT_OFFSET], RESULT_OFFSET);
    if (std::holds_alternative<Error>(resultField))
    {
        response.error.emplace(std::get<Error>(resultField));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(resultField));

    size_t offset = RETURN_PARAMS_OFFSET;
    if (data.size() > offset)
    {
        uint8_t hasReturn = data[offset];

        ParsedField returnParamField;
        returnParamField.identifier = "return-parameters";
        returnParamField.name = "Return-Parameters";
        returnParamField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, 1);
        response.fields.values.push_back(returnParamField);
        offset++;

        if (hasReturn == 0x01)
        {
            if (offset >= data.size())
            {
                response.error.emplace(Error{"ACTION-RESPONSE-NORMAL invalido: Get-Data-Result ausente."});
                return response;
            }

            uint8_t choiceTag = data[offset];
            if (choiceTag == 0x00)
            {
                size_t dataEnd = offset + 1;
                auto dataResult = CosemDataParser::parse(data, dataEnd, dataEnd);
                if (std::holds_alternative<Error>(dataResult))
                {
                    response.error.emplace(std::get<Error>(dataResult));
                    return response;
                }
                ParsedField getDataResultField;
                getDataResultField.identifier = "get-data-result";
                getDataResultField.name = "Get-Data-Result";
                getDataResultField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, 1);
                getDataResultField.values.push_back(std::get<ParsedField>(dataResult));
                response.fields.values.push_back(getDataResultField);
                offset = dataEnd;
            }
            else if (choiceTag == 0x01)
            {
                constexpr size_t choiceTagSize = 1;
                if (offset + choiceTagSize >= data.size())
                {
                    response.error.emplace(Error{"ACTION-RESPONSE-NORMAL invalido: Data-Access-Result do choice ausente."});
                    return response;
                }

                size_t darOffset = offset + choiceTagSize;
                auto dar = parseActionResult(data[darOffset], darOffset);
                if (std::holds_alternative<Error>(dar))
                {
                    response.error.emplace(std::get<Error>(dar));
                    return response;
                }

                constexpr size_t choiceFieldSize = choiceTagSize + 1;
                auto f = std::get<ParsedField>(dar);
                f.identifier = "get-data-result";
                f.name = "Get-Data-Result";
                f.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, choiceFieldSize);
                response.fields.values.push_back(f);
                offset += choiceFieldSize;
            }
            else
            {
                response.error.emplace(Error{"Tag de escolha de Get-Data-Result invalida: " + std::to_string(choiceTag)});
                return response;
            }
        }
        else if (hasReturn != 0x00)
        {
            response.error.emplace(Error{"Indicador de Return-Parameters inválido: " + std::to_string(hasReturn)});
            return response;
        }
    }

    if (offset < data.size())
        response.error.emplace(Error{"Erro estrutural: Bytes extras detectados ao final do frame ACTION-RESPONSE-NORMAL."});

    return response;
}

auto ActionResponseParser::verifyWithPblock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-response-with-pblock";
    response.fields.name = "Action-Response-With-Pblock";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + DlmsFrameUtils::DATABLOCK_HEADER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-RESPONSE-WITH-PBLOCK incompleto."});
        return response;
    }

    auto blockResult = parseDataBlockSA(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(blockResult))
        response.error.emplace(std::get<Error>(blockResult));
    else
        response.fields.values.push_back(std::get<ParsedField>(blockResult));

    return response;
}

auto ActionResponseParser::verifyNextPblock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-response-next-pblock";
    response.fields.name = "Action-Response-Next-Pblock";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + BLOCK_NUMBER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-RESPONSE-NEXT-PBLOCK incompleto."});
        return response;
    }

    ParsedField bnField;
    bnField.identifier = "block-number";
    bnField.name = "Block-Number";
    bnField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, PAYLOAD_OFFSET, BLOCK_NUMBER_SIZE);
    response.fields.values.push_back(bnField);

    if (data.size() > minimumSize)
        response.error.emplace(Error{"Erro estrutural: Bytes extras detectados ao final do frame ACTION-RESPONSE-NEXT-PBLOCK."});

    return response;
}
