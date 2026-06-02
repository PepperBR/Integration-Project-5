#include "core/VerifierCOSEM.h"

static std::string toHexString(const unsigned char *bytes, int length)
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
}

auto VerifierCOSEM::verifyCOSEM(std::vector<uint8_t> data) -> VerifyFrameResponse
{
    if (data.size() < 4)
    {
        VerifyFrameResponse response;
        response.valid = false;
        ValidationError err;

        err.offset = 0;
        err.message = "Frame muito curto para ser uma APDU DLMS válida.";
        err.found = toHexString((unsigned char *)data.data(), static_cast<int>(data.size()));
        response.errors.push_back(err);

        return response;
    }

    // Detect the core service group (GET, SET, or ACTION)
    auto commandType = selectTypeCommand(data[0]);

    if (commandType == Enums::UNKNOWN)
    {
        VerifyFrameResponse response;
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = "Tag de serviço APDU desconhecida ou não suportada.";
        err.found = toHexString((unsigned char *)&data[0], 1);
        response.errors.push_back(err);
        return response;
    }

    return selectTypeVerifier(commandType, data);
}

auto VerifierCOSEM::selectTypeCommand(uint8_t type_command) -> Enums
{
    switch (type_command)
    {
    case 0xC0:
    case 0xC4:
        return Enums::GET;
    case 0xC1:
    case 0xC5:
        return Enums::SET;
    case 0xC6:
    case 0xC7:
        return Enums::ACTION;
    default:
        return Enums::UNKNOWN;
    }
}

auto VerifierCOSEM::selectTypeVerifier(Enums type_command, const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    switch (type_command)
    {
    case Enums::GET: {
        Get_Verifier get_verifier;
        return get_verifier.typeVerifier(data);
    }
    case Enums::SET: {
        Set_Verifier set_verifier;
        return set_verifier.typeVerifier(data);
    }
    case Enums::ACTION: {
        Actions_Verifier actions_verifier;
        return actions_verifier.typeVerifier(data);
    }
    default:
        VerifyFrameResponse response;
        response.valid = false;
        return response;
    }
}