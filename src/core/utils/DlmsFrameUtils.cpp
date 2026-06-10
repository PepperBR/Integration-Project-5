#include "core/utils/DlmsFrameUtils.h"

#include <iomanip>
#include <sstream>

auto DlmsFrameUtils::bytes_to_hex(const std::vector<uint8_t> &data, size_t start, size_t len) -> std::string
{
    std::ostringstream oss;
    for (size_t i = start; i < start + len && i < data.size(); ++i)
    {
        if (i != start)
            oss << ' ';
        oss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(data[i]);
    }
    return oss.str();
}

auto DlmsFrameUtils::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    auto headerResult = ParseHeader::parse_header(data, APDU_SERVICE_TYPE_OFFSET);

    if (std::holds_alternative<Error>(headerResult))
    {
        response.error.emplace(std::get<Error>(headerResult));
        return false;
    }
    response.fields.values.push_back(std::get<ParsedField>(headerResult));
    return true;
}

auto DlmsFrameUtils::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    if (offset + DATABLOCK_HEADER_SIZE > data.size())
        return Error{"DataBlock-SA incompleto ou sem definicao de tamanho."};

    constexpr size_t lastBlockRelOffset = 0;
    constexpr size_t blockNumberRelOffset = lastBlockRelOffset + DATABLOCK_LAST_BLOCK_SIZE;
    constexpr size_t lengthRelOffset = blockNumberRelOffset + DATABLOCK_BLOCK_NUMBER_SIZE;
    constexpr size_t rawDataRelOffset = lengthRelOffset + DATABLOCK_LENGTH_SIZE;

    const size_t lastBlockAbs = offset + lastBlockRelOffset;
    const size_t blockNumberAbs = offset + blockNumberRelOffset;
    const size_t lengthAbs = offset + lengthRelOffset;
    const size_t rawDataAbs = offset + rawDataRelOffset;

    bool lastBlock = data[lastBlockAbs] != 0x00;
    uint32_t blockNumber = read_uint32_be(data, blockNumberAbs);
    uint8_t declaredBlockLen = data[lengthAbs];
    size_t actualBlockLen = data.size() - rawDataAbs;

    if (actualBlockLen != declaredBlockLen)
    {
        return Error{"Erro estrutural no DataBlock-SA: Tamanho declarado (" + std::to_string(declaredBlockLen) +
                     " bytes) difere dos bytes reais recebidos (" + std::to_string(actualBlockLen) + " bytes)."};
    }

    ParsedField lastBlockField;
    lastBlockField.identifier = "last-block";
    lastBlockField.name = "Last-Block";
    lastBlockField.value_bytes = bytes_to_hex(data, lastBlockAbs, DATABLOCK_LAST_BLOCK_SIZE);

    ParsedField blockNumberField;
    blockNumberField.identifier = "block-number";
    blockNumberField.name = "Block-Number";
    blockNumberField.value_bytes = bytes_to_hex(data, blockNumberAbs, DATABLOCK_BLOCK_NUMBER_SIZE);

    ParsedField rawDataField;
    rawDataField.identifier = "raw-data";
    rawDataField.name = "Raw-Data";
    rawDataField.value_bytes = bytes_to_hex(data, rawDataAbs, actualBlockLen);

    ParsedField field;
    field.identifier = "datablock-sa";
    field.name = "DataBlock-SA";
    field.value_bytes = bytes_to_hex(data, offset, data.size() - offset);
    field.values.push_back(std::move(lastBlockField));
    field.values.push_back(std::move(blockNumberField));
    field.values.push_back(std::move(rawDataField));
    return field;
}

auto DlmsFrameUtils::read_uint32_be(const std::vector<uint8_t> &data, size_t offset) -> uint32_t
{
    return (static_cast<uint32_t>(data[offset]) << 24) | (static_cast<uint32_t>(data[offset + 1]) << 16) |
           (static_cast<uint32_t>(data[offset + 2]) << 8) | static_cast<uint32_t>(data[offset + 3]);
}
