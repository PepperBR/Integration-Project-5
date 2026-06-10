#include "core/CommandTypes/SET/REQUEST/SetRequestParser.h"
#include "core/enums.h"
#include "core/utils/CosemDataParser.h"
#include "core/utils/CosemDescriptorParser.h"
#include "core/utils/DlmsFrameUtils.h"

#include <string>
#include <variant>

static constexpr size_t PAYLOAD_OFFSET = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
static constexpr size_t DESCRIPTOR_SIZE = CosemDescriptorParser::DESCRIPTOR_SIZE;
static constexpr size_t SELECTION_OFFSET = PAYLOAD_OFFSET + DESCRIPTOR_SIZE;
static constexpr size_t DATA_OFFSET = SELECTION_OFFSET + 1;
static constexpr size_t BLOCK_NUMBER_SIZE = DlmsFrameUtils::DATABLOCK_BLOCK_NUMBER_SIZE;

auto SetRequestParser::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    return DlmsFrameUtils::buildHeader(data, response);
}

auto SetRequestParser::verify(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-request";
    response.fields.name = "Set-Request";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    constexpr size_t MIN_FRAME_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
    if (data.size() < MIN_FRAME_SIZE)
    {
        response.error.emplace(Error{"Frame SET-REQUEST muito curto."});
        return response;
    }

    switch (data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyWithFirstDatablock(data);
    case 0x03:
        return verifyWithDatablock(data);
    case 0x04:
        response.error.emplace(Error{"SET-REQUEST-WITH-LIST não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    case 0x05:
        response.error.emplace(
            Error{"SET-REQUEST-WITH-LIST-AND-FIRST-DATA-BLOCK não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    default:
        response.error.emplace(Error{"Sub-tipo de SET-REQUEST desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    }
}

auto SetRequestParser::parseCosemAttributeDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return CosemDescriptorParser::parseCosemAttributeDescriptor(data, offset);
}

auto SetRequestParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return DlmsFrameUtils::parseDataBlockSA(data, offset);
}

auto SetRequestParser::verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-request-normal";
    response.fields.name = "Set-Request-Normal";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = DATA_OFFSET;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"SET-REQUEST-NORMAL incompleto."});
        return response;
    }

    auto descResult = parseCosemAttributeDescriptor(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(descResult))
    {
        response.error.emplace(std::get<Error>(descResult));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(descResult));

    uint8_t hasSelection = data[SELECTION_OFFSET];
    ParsedField selField;
    selField.identifier = "access-selection";
    selField.name = "Access-Selection";
    selField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, SELECTION_OFFSET, 1);
    response.fields.values.push_back(selField);

    if (hasSelection == 0x01 && DATA_OFFSET >= data.size())
    {
        response.error.emplace(Error{"SET-REQUEST-NORMAL invalido: Selective-Access-Descriptor ausente."});
        return response;
    }

    if (DATA_OFFSET >= data.size())
    {
        response.error.emplace(Error{"SET-REQUEST-NORMAL invalido: Valor a ser escrito (Data value) ausente."});
        return response;
    }

    size_t dataEnd = DATA_OFFSET;
    auto dataResult = CosemDataParser::parse(data, dataEnd, dataEnd);
    if (std::holds_alternative<Error>(dataResult))
    {
        response.error.emplace(std::get<Error>(dataResult));
        return response;
    }
    ParsedField dataField;
    dataField.identifier = "data";
    dataField.name = "Data";
    dataField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DATA_OFFSET, dataEnd - DATA_OFFSET);
    dataField.values.push_back(std::get<ParsedField>(dataResult));
    response.fields.values.push_back(dataField);

    return response;
}

auto SetRequestParser::verifyWithFirstDatablock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-request-with-first-datablock";
    response.fields.name = "Set-Request-With-First-Datablock";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = DATA_OFFSET + DlmsFrameUtils::DATABLOCK_HEADER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"SET-REQUEST-WITH-FIRST-DATABLOCK incompleto."});
        return response;
    }

    auto descResult = parseCosemAttributeDescriptor(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(descResult))
    {
        response.error.emplace(std::get<Error>(descResult));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(descResult));

    uint8_t hasSelection = data[SELECTION_OFFSET];
    ParsedField selField;
    selField.identifier = "access-selection";
    selField.name = "Access-Selection";
    selField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, SELECTION_OFFSET, 1);
    response.fields.values.push_back(selField);

    auto blockResult = parseDataBlockSA(data, DATA_OFFSET);
    if (std::holds_alternative<Error>(blockResult))
        response.error.emplace(std::get<Error>(blockResult));
    else
        response.fields.values.push_back(std::get<ParsedField>(blockResult));

    return response;
}

auto SetRequestParser::verifyWithDatablock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "set-request-with-datablock";
    response.fields.name = "Set-Request-With-Datablock";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + DlmsFrameUtils::DATABLOCK_HEADER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"SET-REQUEST-WITH-DATABLOCK incompleto."});
        return response;
    }

    auto blockResult = parseDataBlockSA(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(blockResult))
        response.error.emplace(std::get<Error>(blockResult));
    else
        response.fields.values.push_back(std::get<ParsedField>(blockResult));

    return response;
}
