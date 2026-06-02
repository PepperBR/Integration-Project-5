#include "core/VerifierCOSEM.h"
#include "core/CommandTypes/GET/GET_Verifier.h" // Assuming this is where GET_Verifier lives
// #include "core/CommandTypes/SET/SET_Verifier.h"      // Ready for later
// #include "core/CommandTypes/ACTIONS/ACTION_Verifier.h" // Ready for later

#include <iomanip>
#include <sstream>

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

    if (commandType == COSEMCommandType::UNKNOWN)
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

auto VerifierCOSEM::selectTypeCommand(uint8_t type_command) -> COSEMCommandType
{
    switch (type_command)
    {
    case 0xC0:
    case 0xC4:
        return COSEMCommandType::GET;
    case 0xC1:
    case 0xC5:
        return COSEMCommandType::SET;
    case 0xC3:
    case 0xC7:
        return COSEMCommandType::ACTION;
    default:
        return COSEMCommandType::UNKNOWN;
    }
}

auto VerifierCOSEM::selectTypeVerifier(COSEMCommandType type_command, const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    switch (type_command)
    {
    case COSEMCommandType::GET: {
        // Tier 2: Hands off the vector to the GET director
        Get_Verifier get_verifier;
        return get_verifier.typeVerifier(data);
    }
    case COSEMCommandType::SET: {
        // TODO: Return set_verifier.verify(data); when ready
        VerifyFrameResponse response;
        response.valid = false;
        return response;
    }
    case COSEMCommandType::ACTION: {
        // TODO: Return action_verifier.verify(data); when ready
        VerifyFrameResponse response;
        response.valid = false;
        return response;
    }
    default:
        VerifyFrameResponse response;
        response.valid = false;
        return response;
    }
}