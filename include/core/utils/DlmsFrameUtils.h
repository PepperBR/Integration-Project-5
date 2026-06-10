#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

class DlmsFrameUtils
{
public:
    static constexpr size_t APDU_TAG_OFFSET = 0;
    static constexpr size_t APDU_SERVICE_TYPE_OFFSET = 1;
    static constexpr size_t APDU_INVOKE_ID_OFFSET = 2;
    static constexpr size_t APDU_HEADER_SIZE = 2;
    static constexpr size_t APDU_PAYLOAD_OFFSET = 3;

    static constexpr size_t DATABLOCK_LAST_BLOCK_SIZE = 1;
    static constexpr size_t DATABLOCK_BLOCK_NUMBER_SIZE = 4;
    static constexpr size_t DATABLOCK_LENGTH_SIZE = 1;
    static constexpr size_t DATABLOCK_HEADER_SIZE = DATABLOCK_LAST_BLOCK_SIZE + DATABLOCK_BLOCK_NUMBER_SIZE + DATABLOCK_LENGTH_SIZE;

    static auto bytes_to_hex(const std::vector<uint8_t> &data, size_t start, size_t len) -> std::string;
    static auto buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool;
    static auto parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
    static auto read_uint32_be(const std::vector<uint8_t> &data, size_t offset) -> uint32_t;
};
