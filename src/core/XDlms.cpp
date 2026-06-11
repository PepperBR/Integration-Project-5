#include "core/XDlms.h"
#include "core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.h"
#include "core/CommandTypes/ACTIONS/RESPONSE/ActionResponseParser.h"

#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"
#include "core/CommandTypes/GET/RESPONSE/GetResponseParser.h"

#include "core/CommandTypes/SET/REQUEST/SetRequestParser.h"
#include "core/CommandTypes/SET/RESPONSE/SetResponseParser.h"

#include "core/CommonVerifierTypes.h"
#include "core/utils/DlmsFrameUtils.h"

auto XDlms::decode(const std::vector<uint8_t> &data) -> FrameResponse
{
    constexpr size_t MIN_APDU_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET + 1;

    if (data.size() < MIN_APDU_SIZE)
    {
        FrameResponse response;
        Error err;
        err.message = "Frame muito curto para ser uma APDU DLMS válida.";
        response.error.emplace(err);
        return response;
    }

    constexpr size_t TAG_OFFSET = DlmsFrameUtils::APDU_TAG_OFFSET;
    auto commandType = static_cast<XDlmsApduTag>(data[TAG_OFFSET]);

    constexpr int parserOffset = static_cast<int>(DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET);

    auto inner = verify_command(commandType, data);

    FrameResponse response;
    response.fields.identifier = "XDLMS-APDU::CHOICE";
    response.fields.name = "XDLMS-APDU";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_TAG_OFFSET, 1);

    if (inner.error.has_value())
    {
        response.error = inner.error;
        return response;
    }

    inner.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);
    response.fields.values.push_back(std::move(inner.fields));
    return response;
}

auto XDlms::verify_command(XDlmsApduTag commandType, const std::vector<uint8_t> &data) -> FrameResponse
{
    switch (commandType)
    {
    case XDlmsApduTag::GET_REQUEST:
        return GetRequestParser::verify(data);
    case XDlmsApduTag::GET_RESPONSE:
        return GetResponseParser::verify(data);
    case XDlmsApduTag::SET_REQUEST:
        return SetRequestParser::verify(data);
    case XDlmsApduTag::SET_RESPONSE:
        return SetResponseParser::verify(data);
    case XDlmsApduTag::ACTION_REQUEST:
        return ActionRequestParser::verify(data);
    case XDlmsApduTag::ACTION_RESPONSE:
        return ActionResponseParser::verify(data);
    default:
        FrameResponse response;
        Error err;
        err.message = "Tipo de Comando desconhecido ou não suportado.";
        response.error.emplace(err);
        return response;
    }
}

auto XDlms::to_hex_string(const unsigned char *bytes, int length) -> std::string
{
    std::vector<uint8_t> tmp(bytes, bytes + length);
    return DlmsFrameUtils::bytes_to_hex(tmp, 0, static_cast<size_t>(length));
}
