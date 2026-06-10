#pragma once

#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

class ActionResponseParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> FrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyWithPblock(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyNextPblock(const std::vector<uint8_t> &data) -> FrameResponse;

    static auto parseActionResult(uint8_t value, size_t offset) -> std::variant<ParsedField, Error>;
    static auto parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool;
};
