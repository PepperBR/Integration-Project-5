#include "core/CommandTypes/GET/RESPONSE/GetResponseParser.h"
#include "core/enums.h"
#include "core/utils/CosemDataParser.h"
#include "core/utils/DlmsFrameUtils.h"

#include <iomanip>
#include <sstream>
#include <string>

static constexpr size_t PAYLOAD_OFFSET = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
static constexpr size_t BLOCK_NUMBER_SIZE = DlmsFrameUtils::DATABLOCK_BLOCK_NUMBER_SIZE;
static constexpr size_t LAST_BLOCK_SIZE = DlmsFrameUtils::DATABLOCK_LAST_BLOCK_SIZE;
static constexpr size_t RESULT_TAG_SIZE = 1;
static constexpr size_t DATABLOCK_G_HEADER_SIZE = LAST_BLOCK_SIZE + BLOCK_NUMBER_SIZE + RESULT_TAG_SIZE;

auto GetResponseParser::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    return DlmsFrameUtils::buildHeader(data, response);
}

auto GetResponseParser::verify(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "get-response";
    response.fields.name = "Get-Response";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    constexpr size_t MIN_FRAME_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
    if (data.size() < MIN_FRAME_SIZE)
    {
        response.error.emplace(Error{"Frame GET-RESPONSE muito curto."});
        return response;
    }

    switch (data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyWithDatablock(data);
    case 0x03:
        response.error.emplace(Error{"GET-RESPONSE-WITH-LIST desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    default:
        response.error.emplace(Error{"Sub-tipo de GET-RESPONSE desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    }
}

auto GetResponseParser::verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "get-response-normal";
    response.fields.name = "Get-Response-Normal";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + 1;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"GET-RESPONSE-NORMAL incompleto."});
        return response;
    }

    auto result = parseGetDataResult(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(result))
    {
        response.error.emplace(std::get<Error>(result));
        return response;
    }

    response.fields.values.push_back(std::get<ParsedField>(result));
    return response;
}

auto GetResponseParser::verifyWithDatablock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "get-response-with-datablock";
    response.fields.name = "Get-Response-With-Datablock";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    // Mínimo: header(3) + datablock_g_header(6) = 9
    constexpr size_t minimumSize = PAYLOAD_OFFSET + DATABLOCK_G_HEADER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"GET-RESPONSE-WITH-DATABLOCK incompleto."});
        return response;
    }

    auto result = parseDataBlockG(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(result))
        response.error.emplace(std::get<Error>(result));
    else
        response.fields.values.push_back(std::get<ParsedField>(result));

    return response;
}

auto GetResponseParser::parseDataAccessResult(uint8_t value, size_t /*offset*/) -> std::variant<ParsedField, Error>
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

auto GetResponseParser::parseGetDataResult(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    if (offset >= data.size())
        return Error{"Get-Data-Result ausente."};

    constexpr uint8_t CHOICE_DATA    = 0x00;
    constexpr uint8_t CHOICE_ERROR   = 0x01;
    constexpr size_t  CHOICE_TAG_SIZE = 1;
    constexpr size_t  DAR_SIZE        = 1;

    uint8_t choiceTag = data[offset];

    ParsedField field;
    field.identifier = "result";
    field.name       = "Get-Data-Result";

    if (choiceTag == CHOICE_DATA)
    {
        // Decodifica a estrutura COSEM Data de forma recursiva e hierárquica
        size_t dataEnd = offset + CHOICE_TAG_SIZE;
        auto dataResult = CosemDataParser::parse(data, dataEnd, dataEnd);

        if (std::holds_alternative<Error>(dataResult))
            return std::get<Error>(dataResult);

        auto dataField = std::get<ParsedField>(dataResult);
        field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, CHOICE_TAG_SIZE); // choice tag (0x00 = Data)
        field.values.push_back(std::move(dataField));
        return field;
    }
    else if (choiceTag == CHOICE_ERROR)
    {
        size_t darOffset = offset + CHOICE_TAG_SIZE;
        if (darOffset >= data.size())
            return Error{"Data-Access-Result ausente após tag de choice."};

        auto darResult = parseDataAccessResult(data[darOffset], darOffset);
        if (std::holds_alternative<Error>(darResult))
            return darResult;

        auto darField = std::get<ParsedField>(darResult);
        field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, CHOICE_TAG_SIZE); // choice tag (0x01 = Error)
        field.values.push_back(std::move(darField));
        return field;
    }

    return Error{"Tag de Get-Data-Result inválida: " + std::to_string(choiceTag)};
}

auto GetResponseParser::parseDataBlockG(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    constexpr size_t resultTagRelOffset = LAST_BLOCK_SIZE + BLOCK_NUMBER_SIZE;
    constexpr size_t rawDataRelOffset = resultTagRelOffset + RESULT_TAG_SIZE;

    if (offset + DATABLOCK_G_HEADER_SIZE > data.size())
        return Error{"DataBlock-G incompleto."};

    const size_t lastBlockAbs = offset;
    const size_t blockNumberAbs = offset + LAST_BLOCK_SIZE;
    const size_t resultTagAbs = offset + resultTagRelOffset;
    const size_t rawDataAbs = offset + rawDataRelOffset;

    bool lastBlock = data[lastBlockAbs] != 0x00;
    uint32_t blockNumber = DlmsFrameUtils::read_uint32_be(data, blockNumberAbs);
    uint8_t resultTag = data[resultTagAbs];

    ParsedField lastBlockField;
    lastBlockField.identifier = "last-block";
    lastBlockField.name = "Last-Block";
    lastBlockField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, lastBlockAbs, LAST_BLOCK_SIZE);

    ParsedField blockNumberField;
    blockNumberField.identifier = "block-number";
    blockNumberField.name = "Block-Number";
    blockNumberField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, blockNumberAbs, BLOCK_NUMBER_SIZE);

    ParsedField resultField;
    resultField.identifier = "result";
    resultField.name = "Result";

    constexpr uint8_t RESULT_DATA = 0x00;
    constexpr uint8_t RESULT_ERROR = 0x01;

    if (resultTag == RESULT_DATA)
    {
        size_t rawLen = data.size() - rawDataAbs;
        resultField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, resultTagAbs, RESULT_TAG_SIZE); // result tag
    }
    else if (resultTag == RESULT_ERROR)
    {
        if (rawDataAbs > data.size())
            return Error{"Data-Access-Result ausente em DataBlock-G."};
        auto dar = parseDataAccessResult(data[rawDataAbs], rawDataAbs);
        if (std::holds_alternative<Error>(dar))
            return dar;
        auto darField = std::get<ParsedField>(dar);
        resultField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, resultTagAbs, RESULT_TAG_SIZE); // result tag
        resultField.values.push_back(std::move(darField));
    }
    else
    {
        return Error{"Tag de result em DataBlock-G inválida: " + std::to_string(resultTag)};
    }

    ParsedField field;
    field.identifier = "datablock-g";
    field.name = "DataBlock-G";
    field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, LAST_BLOCK_SIZE + BLOCK_NUMBER_SIZE + RESULT_TAG_SIZE); // last-block+block-num+result-tag
    field.values.push_back(std::move(lastBlockField));
    field.values.push_back(std::move(blockNumberField));
    field.values.push_back(std::move(resultField));

    return field;
}
