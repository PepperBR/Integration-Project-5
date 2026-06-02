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
#include "core/COSEMCommandType.h"
#include "core/CommandTypes/GET/GET_enums.h"
#include "core/CommonVerifierTypes.h"

class GET_RESPONSE_VERIFIER
{
public:
    auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};