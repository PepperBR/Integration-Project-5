#pragma once

#include <iomanip>
#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "Enums.h"
#include "GXAPDU.h"
#include "GXDLMS.h"
#include "GXDLMSTranslator.h"
#include "GXReplyData.h"
#include "core/CommandTypes/ACTIONS/ACTIONS_Verifier.h"
#include "core/CommandTypes/GET/GET_Verifier.h"
#include "core/CommandTypes/SET/SET_Verifier.h"
#include "core/CommonVerifierTypes.h"

// Verify if is a valid command

class VerifierCOSEM
{
public:
    auto verifyCOSEM(std::vector<uint8_t> data) -> VerifyFrameResponse;
    auto selectTypeCommand(uint8_t type_command) -> Enums;
    auto selectTypeVerifier(Enums type_command, const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};