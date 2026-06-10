#pragma once

#include "core/CommonVerifierTypes.h"

#include <cstdint>
#include <variant>
#include <vector>

class CosemDescriptorParser
{
public:
    static constexpr size_t CLASS_ID_SIZE = 2;
    static constexpr size_t OBIS_SIZE = 6;
    static constexpr size_t ID_FIELD_SIZE = 1;
    static constexpr size_t DESCRIPTOR_SIZE = CLASS_ID_SIZE + OBIS_SIZE + ID_FIELD_SIZE;

    static constexpr size_t CLASS_ID_REL_OFFSET = 0;
    static constexpr size_t OBIS_REL_OFFSET = CLASS_ID_REL_OFFSET + CLASS_ID_SIZE;
    static constexpr size_t ID_REL_OFFSET = OBIS_REL_OFFSET + OBIS_SIZE;

    static auto parseCosemAttributeDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;

    static auto parseCosemMethodDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;

private:
    static auto parseDescriptor(const std::vector<uint8_t> &data, size_t offset, const char *idIdentifier, const char *idName,
                                const char *outerIdentifier, const char *outerName) -> std::variant<ParsedField, Error>;
};
