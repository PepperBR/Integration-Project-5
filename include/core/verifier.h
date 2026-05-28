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

struct ParsedField
{
    std::string name;
    int offset;
    int length;
    std::string value;
    std::string description;
};

struct ValidationError
{
    int offset;
    std::string message;
    std::string found;
};

struct VerifyFrameResponse
{
    bool valid;
    std::list<ParsedField> fields;
    std::list<ValidationError> errors;
};

class Verifier
{
public:
    auto validateData(std::vector<uint8_t> data) -> VerifyFrameResponse;
};