#pragma once

#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

class SetRequestParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> FrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyWithFirstDatablock(const std::vector<uint8_t> &data) -> FrameResponse;
    static auto verifyWithDatablock(const std::vector<uint8_t> &data) -> FrameResponse;

    static auto parseCosemAttributeDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool;
};
