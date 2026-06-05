#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"

#include <variant>
#include <vector>

class GetRequestParser
{
private:
    static auto Cosem_Attribute_Descriptor(const std::vector<uint8_t> &data) -> std::variant<ParsedField, ValidationError>;

public:
    static auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};
