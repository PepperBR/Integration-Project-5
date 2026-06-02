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
#include "core/CommonVerifierTypes.h"
#include "core/Enums.h"

class SET_RESPONSE_VERIFIER
{
public:
    auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};