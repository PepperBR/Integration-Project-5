#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"

#include <variant>
#include <vector>

class SetResponseParser
{
private:
    static auto decodeInvokeField(uint8_t value) -> InvokeIdAndPriority;
    static auto parseDataAccessResult(uint8_t value) -> std::variant<ParsedField, ValidationError>;

public:
    static auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};
