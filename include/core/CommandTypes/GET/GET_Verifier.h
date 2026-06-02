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
#include "core/CommandTypes/GET/GET_REQUEST_Verifier.h"
#include "core/CommonVerifierTypes.h"

// verify what kind of Get we are working with and calling the right verify

class Get_Verifier
{
public:
    auto typeVerifier(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};