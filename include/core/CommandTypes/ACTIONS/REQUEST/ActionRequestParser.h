#pragma once

#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

class ActionRequestParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> FrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyNextPblock(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyWithFirstPblock(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyWithPblock(const std::vector<uint8_t> &data) -> FrameResponse;

    static auto parseCosemMethodDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool;
};
