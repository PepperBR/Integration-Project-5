#pragma once

#include <iomanip>
#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "GXAPDU.h"
#include "GXDLMS.h"
#include "GXDLMSTranslator.h"
#include "GXReplyData.h"
#include "core/CommandTypes/ACTIONS/ACTIONS_REQUEST_VERIFIER.h"
#include "core/CommandTypes/ACTIONS/ACTIONS_RESPONSE_VERIFIER.h"
#include "core/CommonVerifierTypes.h"
#include "core/Enums.h"

class Actions_Verifier
{
public:
    auto typeVerifier(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};