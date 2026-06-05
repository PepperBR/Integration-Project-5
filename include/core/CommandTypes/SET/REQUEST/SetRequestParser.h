#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

class SetRequestParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithFirstDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithListAndFirstDatablock(const std::vector<uint8_t> &data) -> VerifyFrameResponse;

    static auto parseCosemAttributeDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>;
    static auto parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>;
    static auto buildHeader(const std::vector<uint8_t> &data, VerifyFrameResponse &response) -> bool;
};
