#include "core/XDlms.h"
#include "core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.h"
#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"
#include <iomanip>
#include <list>
#include <sstream>
auto XDlms::decode(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    if (data.size() < 4)
    {
        VerifyFrameResponse response;
        response.valid = false;
        ValidationError err;

        err.offset = 0;
        err.message = "Frame muito curto para ser uma APDU DLMS válida.";
        err.found = to_hex_string((unsigned char *)data.data(), static_cast<int>(data.size()));
        response.errors.push_back(err);

        return response;
    }

    auto commandType = identifier_x_dlms_data_type(data[0]);

    return verify_command(commandType, data);
};

auto XDlms::verify_command(XDlmsDataType commandType, const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    switch (commandType)
    {
    case XDlmsDataType::GET_REQUEST:
        return GetRequestParser::verify(data);
        // case XDlmsDataType::GET_RESPONSE:
        //     return GetResponseParser::verify(data);
        // case XDlmsDataType::SET_REQUEST:
        //     return SetRequestParser::verify(data);
        // case XDlmsDataType::SET_RESPONSE:
        //     return SetResponseParser::verify(data);
    case XDlmsDataType::ACTION_REQUEST:
        return ActionRequestParser::verify(data);
    // case XDlmsDataType::ACTION_RESPONSE:
    //     return ActionResponseParser::verify(data);
    default:
        VerifyFrameResponse response;
        response.valid = false;
        ValidationError err;

        err.offset = 0;
        err.message = "Tipo de Comando desconhecido ou não suportado.";
        err.found = to_hex_string((unsigned char *)data.data(), 3);
        response.errors.push_back(err);

        return response;
    }
}

auto XDlms::to_hex_string(const unsigned char *bytes, int length) -> std::string
{
    std::ostringstream oss;
    for (int i = 0; i < length; ++i)
    {
        oss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)bytes[i] << " ";
    }
    std::string res = oss.str();
    if (!res.empty())
    {
        res.pop_back();
    }
    return res;
};

auto XDlms::identifier_x_dlms_data_type(uint8_t data) -> XDlmsDataType
{
    // Realiza o cast do byte puro diretamente para o enum fortemente tipado
    auto tag = static_cast<XDlmsApduTag>(data);

    switch (tag)
    {
    case XDlmsApduTag::GET_REQUEST:
        return XDlmsDataType::GET_REQUEST;
    case XDlmsApduTag::GET_RESPONSE:
        return XDlmsDataType::GET_RESPONSE;
    case XDlmsApduTag::SET_REQUEST:
        return XDlmsDataType::SET_REQUEST;
    case XDlmsApduTag::SET_RESPONSE:
        return XDlmsDataType::SET_RESPONSE;
    case XDlmsApduTag::ACTION_REQUEST:
        return XDlmsDataType::ACTION_REQUEST;
    case XDlmsApduTag::ACTION_RESPONSE:
        return XDlmsDataType::ACTION_RESPONSE;
    case XDlmsApduTag::EXCEPTION_RESPONSE:
        return XDlmsDataType::EXCEPTION_RESPONSE;
    case XDlmsApduTag::ACCESS_REQUEST:
        return XDlmsDataType::ACCESS_REQUEST;
    case XDlmsApduTag::ACCESS_RESPONSE:
        return XDlmsDataType::ACCESS_RESPONSE;
    case XDlmsApduTag::EVENT_NOTIFICATION_REQUEST:
        return XDlmsDataType::EVENT_NOTIFICATION_REQUEST;
    case XDlmsApduTag::DATA_NOTIFICATION:
        return XDlmsDataType::DATA_NOTIFICATION;
    case XDlmsApduTag::GENERAL_BLOCK_TRANSFER:
        return XDlmsDataType::GENERAL_BLOCK_TRANSFER;
    case XDlmsApduTag::GENERAL_CIPHERING:
        return XDlmsDataType::GENERAL_CIPHERING;
    case XDlmsApduTag::GENERAL_SIGNING:
        return XDlmsDataType::GENERAL_SIGNING;
    default:
        return XDlmsDataType::UNKNOWN;
    }
};