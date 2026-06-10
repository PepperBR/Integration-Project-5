#pragma once

#include "core/CommonVerifierTypes.h"

#include <cstdint>
#include <variant>
#include <vector>

class CosemDataParser
{
public:
    static auto parse(const std::vector<uint8_t> &data, size_t offset, size_t &endOffset) -> std::variant<ParsedField, Error>;

private:
    static auto parseFixed(const std::vector<uint8_t> &data, size_t offset, const char *identifier, const char *name, size_t valueSize,
                           size_t &endOffset) -> std::variant<ParsedField, Error>;

    static auto parseLengthPrefixed(const std::vector<uint8_t> &data, size_t offset, const char *identifier, const char *name, size_t &endOffset)
        -> std::variant<ParsedField, Error>;

    static auto parseSequence(const std::vector<uint8_t> &data, size_t offset, const char *identifier, const char *name, size_t &endOffset)
        -> std::variant<ParsedField, Error>;
};
