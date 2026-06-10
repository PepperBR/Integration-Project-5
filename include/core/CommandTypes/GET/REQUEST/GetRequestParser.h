#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

class GetRequestParser
{
public:
    static auto verify(const std::vector<uint8_t> &data, int offset) -> FrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data, int offset) -> FrameResponse;
    static auto verifyNext(const std::vector<uint8_t> &data) -> FrameResponse;

    static auto Cosem_Attribute_Descriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool;
};
