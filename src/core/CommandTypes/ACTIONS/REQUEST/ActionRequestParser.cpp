#include "core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.h"
#include "core/enums.h"
#include "core/utils/CosemDataParser.h"
#include "core/utils/CosemDescriptorParser.h"
#include "core/utils/DlmsFrameUtils.h"

#include <string>
#include <variant>

static constexpr size_t PAYLOAD_OFFSET = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
static constexpr size_t DESCRIPTOR_SIZE = CosemDescriptorParser::DESCRIPTOR_SIZE;
static constexpr size_t PARAMS_OFFSET = PAYLOAD_OFFSET + DESCRIPTOR_SIZE;
static constexpr size_t BLOCK_NUMBER_SIZE = DlmsFrameUtils::DATABLOCK_BLOCK_NUMBER_SIZE;

auto ActionRequestParser::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    return DlmsFrameUtils::buildHeader(data, response);
}

auto ActionRequestParser::verify(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-request";
    response.fields.name = "Action-Request";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    constexpr size_t MIN_FRAME_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
    if (data.size() < MIN_FRAME_SIZE)
    {
        response.error.emplace(Error{"Frame ACTION-REQUEST muito curto."});
        return response;
    }

    switch (data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyNextPblock(data);
    case 0x03:
        response.error.emplace(Error{"ACTION-REQUEST-LIST não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    case 0x04:
        return verifyWithFirstPblock(data);
    case 0x05:
        response.error.emplace(
            Error{"ACTION-REQUEST-WITH-LIST-AND-FIRST-PBLOCK não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    case 0x06:
        return verifyWithPblock(data);
    default:
        response.error.emplace(Error{"Sub-tipo de ACTION-REQUEST desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    }
}

auto ActionRequestParser::parseCosemMethodDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return CosemDescriptorParser::parseCosemMethodDescriptor(data, offset);
}

auto ActionRequestParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return DlmsFrameUtils::parseDataBlockSA(data, offset);
}

auto ActionRequestParser::verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-request-normal";
    response.fields.name = "Action-Request-Normal";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PARAMS_OFFSET + 1;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-REQUEST-NORMAL incompleto."});
        return response;
    }

    auto descResult = parseCosemMethodDescriptor(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(descResult))
    {
        response.error.emplace(std::get<Error>(descResult));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(descResult));

    uint8_t hasParams = data[PARAMS_OFFSET];
    size_t paramDataOffset = PARAMS_OFFSET + 1;

    ParsedField paramField;
    paramField.identifier = "method-invocation-parameters";
    paramField.name = "Method-Invocation-Parameters";
    paramField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, PARAMS_OFFSET, 1);

    if (hasParams == 0x00)
    {
        response.fields.values.push_back(paramField);
        if (paramDataOffset < data.size())
        {
            response.error.emplace(Error{"Erro estrutural: Bytes extras detectados em um frame ACTION-NORMAL sem parametros."});
        }
    }
    else if (hasParams == 0x01)
    {
        response.fields.values.push_back(paramField);
        if (paramDataOffset >= data.size())
        {
            response.error.emplace(Error{"ACTION-REQUEST-NORMAL invalido: Dados de parametros ausentes após indicador ativo."});
            return response;
        }
        size_t dataEnd = paramDataOffset;
        auto dataResult = CosemDataParser::parse(data, dataEnd, dataEnd);
        if (std::holds_alternative<Error>(dataResult))
        {
            response.error.emplace(std::get<Error>(dataResult));
            return response;
        }
        ParsedField dataField;
        dataField.identifier = "data";
        dataField.name = "Data";
        dataField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, paramDataOffset, dataEnd - paramDataOffset);
        dataField.values.push_back(std::get<ParsedField>(dataResult));
        response.fields.values.push_back(dataField);
    }
    else
    {
        response.error.emplace(Error{"Indicador de Method-Invocation-Parameters inválido: " + std::to_string(hasParams)});
    }

    return response;
}

auto ActionRequestParser::verifyNextPblock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-request-next-pblock";
    response.fields.name = "Action-Request-Next-Pblock";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + BLOCK_NUMBER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-REQUEST-NEXT-PBLOCK incompleto."});
        return response;
    }

    ParsedField bnField;
    bnField.identifier = "block-number";
    bnField.name = "Block-Number";
    bnField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, PAYLOAD_OFFSET, BLOCK_NUMBER_SIZE);
    response.fields.values.push_back(bnField);

    if (data.size() > minimumSize)
        response.error.emplace(Error{"Erro estrutural: Bytes extras detectados ao final do frame ACTION-REQUEST-NEXT-PBLOCK."});

    return response;
}

auto ActionRequestParser::verifyWithFirstPblock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-request-with-first-pblock";
    response.fields.name = "Action-Request-With-First-Pblock";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PARAMS_OFFSET + DlmsFrameUtils::DATABLOCK_HEADER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-REQUEST-WITH-FIRST-PBLOCK incompleto."});
        return response;
    }

    auto descResult = parseCosemMethodDescriptor(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(descResult))
    {
        response.error.emplace(std::get<Error>(descResult));
        return response;
    }
    response.fields.values.push_back(std::get<ParsedField>(descResult));

    auto blockResult = parseDataBlockSA(data, PARAMS_OFFSET);
    if (std::holds_alternative<Error>(blockResult))
        response.error.emplace(std::get<Error>(blockResult));
    else
        response.fields.values.push_back(std::get<ParsedField>(blockResult));

    return response;
}

auto ActionRequestParser::verifyWithPblock(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "action-request-with-pblock";
    response.fields.name = "Action-Request-With-Pblock";
    response.fields.value_bytes =
        DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, data.size() - DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + DlmsFrameUtils::DATABLOCK_HEADER_SIZE;
    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"ACTION-REQUEST-WITH-PBLOCK incompleto."});
        return response;
    }

    auto blockResult = parseDataBlockSA(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<Error>(blockResult))
        response.error.emplace(std::get<Error>(blockResult));
    else
        response.fields.values.push_back(std::get<ParsedField>(blockResult));

    return response;
}
