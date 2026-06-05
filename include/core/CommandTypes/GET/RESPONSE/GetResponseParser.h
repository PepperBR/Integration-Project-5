#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

class GetResponseParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse;

    static auto parseDataAccessResult(uint8_t value, size_t offset) -> std::variant<ParsedField, ValidationError>;
    static auto parseGetDataResult(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>;
    static auto parseDataBlockG(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>;
};
