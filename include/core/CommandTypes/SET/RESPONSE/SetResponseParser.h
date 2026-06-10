#pragma once

#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

class SetResponseParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> FrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyDatablock(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyLastDatablock(const std::vector<uint8_t> &data) -> FrameResponse;

    static auto parseDataAccessResult(uint8_t value, size_t offset) -> std::variant<ParsedField, Error>;
    static auto buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool;
};
