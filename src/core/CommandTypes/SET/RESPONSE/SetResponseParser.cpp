#include "core/CommandTypes/SET/RESPONSE/SetResponseParser.h"
#include "core/enums.h"
#include "core/utils/DlmsFrameUtils.h"

#include <string>

static constexpr size_t PAYLOAD_OFFSET = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
static constexpr size_t BLOCK_NUMBER_SIZE = DlmsFrameUtils::DATABLOCK_BLOCK_NUMBER_SIZE;

auto SetResponseParser::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    return DlmsFrameUtils::buildHeader(data, response);
}

auto SetResponseParser::verify(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-response";
    response.fields.name = "Set-Response";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    constexpr size_t MIN_FRAME_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
    if (data.size() < MIN_FRAME_SIZE)
    {
        response.error.emplace(Error{"Frame SET-RESPONSE muito curto."});
        return response;
    }

    switch (data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyDatablock(data);
    case 0x03:
        return verifyLastDatablock(data);
    case 0x04:
        response.error.emplace(
            Error{"SET-RESPONSE-LAST-DATABLOCK-WITH-LIST não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    case 0x05:
        response.error.emplace(Error{"SET-RESPONSE-WITH-LIST não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    default:
        response.error.emplace(Error{"Sub-tipo de SET-RESPONSE desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    }
}

auto SetResponseParser::parseDataAccessResult(uint8_t value, size_t /*offset*/) -> std::variant<ParsedField, Error>
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
        {
            ParsedField f;
            f.identifier = "data-access-result";
            f.name = "Data-Access-Result";
            f.value_bytes = e.label;
            return f;
        }
    }
    return Error{"Data-Access-Result desconhecido: " + std::to_string(value)};
}

auto SetResponseParser::verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-response-normal";
    response.fields.name = "Set-Response-Normal";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + 1;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"SET-RESPONSE-NORMAL incompleto."});
        return response;
    }

    auto result = parseDataAccessResult(data[PAYLOAD_OFFSET], PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(result))
    {
        response.error.emplace(std::get<Error>(result));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(result));

    if (data.size() > minimumSize)
        response.error.emplace(Error{"Erro estrutural: Bytes extras detectados no final do frame SET-RESPONSE-NORMAL."});

    return response;
}

auto SetResponseParser::verifyDatablock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-response-datablock";
    response.fields.name = "Set-Response-Datablock";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + BLOCK_NUMBER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"SET-RESPONSE-DATABLOCK incompleto."});
        return response;
    }

    uint32_t blockNumber = DlmsFrameUtils::read_uint32_be(data, PAYLOAD_OFFSET);

    ParsedField bnField;
    bnField.identifier = "block-number";
    bnField.name = "Block-Number";
    bnField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, PAYLOAD_OFFSET, BLOCK_NUMBER_SIZE);
    response.fields.values.push_back(bnField);

    if (data.size() > minimumSize)
        response.error.emplace(Error{"Erro estrutural: Bytes extras detectados no final do frame SET-RESPONSE-DATABLOCK."});

    return response;
}

auto SetResponseParser::verifyLastDatablock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-response-last-datablock";
    response.fields.name = "Set-Response-Last-Datablock";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t DAR_SIZE = 1;
    constexpr size_t darOffset = PAYLOAD_OFFSET;
    constexpr size_t blockNumberOffset = darOffset + DAR_SIZE;

    constexpr size_t minimumSize = blockNumberOffset + BLOCK_NUMBER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"SET-RESPONSE-LAST-DATABLOCK incompleto."});
        return response;
    }

    auto result = parseDataAccessResult(data[darOffset], darOffset);
    if (std::holds_alternative<Error>(result))
    {
        response.error.emplace(std::get<Error>(result));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(result));

    uint32_t blockNumber = DlmsFrameUtils::read_uint32_be(data, blockNumberOffset);

    ParsedField bnField;
    bnField.identifier = "block-number";
    bnField.name = "Block-Number";
    bnField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, blockNumberOffset, BLOCK_NUMBER_SIZE);
    response.fields.values.push_back(bnField);

    if (data.size() > minimumSize)
        response.error.emplace(Error{"Erro estrutural: Bytes extras detectados no final do frame SET-RESPONSE-LAST-DATABLOCK."});

    return response;
}
