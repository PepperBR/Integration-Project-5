#include "core/utils/CosemDataParser.h"
#include "core/utils/DlmsFrameUtils.h"

auto CosemDataParser::parseFixed(const std::vector<uint8_t> &data, size_t offset, const char *identifier, const char *name, size_t valueSize,
                                 size_t &endOffset) -> std::variant<ParsedField, Error>
{
    constexpr size_t TAG_SIZE = 1;
    if (offset + TAG_SIZE + valueSize > data.size())
        return Error{"Dados insuficientes para " + std::string(name) + " em offset " + std::to_string(offset) + "."};

    ParsedField valueField;
    valueField.identifier = "value";
    valueField.name = "Value";
    valueField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset + TAG_SIZE, valueSize);

    ParsedField field;
    field.identifier = identifier;
    field.name = name;
    field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, TAG_SIZE);
    field.values.push_back(std::move(valueField));

    endOffset = offset + TAG_SIZE + valueSize;
    return field;
}

auto CosemDataParser::parseLengthPrefixed(const std::vector<uint8_t> &data, size_t offset, const char *identifier, const char *name,
                                          size_t &endOffset) -> std::variant<ParsedField, Error>
{
    constexpr size_t TAG_SIZE = 1;
    constexpr size_t LENGTH_SIZE = 1;

    if (offset + TAG_SIZE + LENGTH_SIZE > data.size())
        return Error{"Dados insuficientes para length de " + std::string(name) + " em offset " + std::to_string(offset) + "."};

    uint8_t length = data[offset + TAG_SIZE];

    if (offset + TAG_SIZE + LENGTH_SIZE + length > data.size())
        return Error{"Payload de " + std::string(name) + " truncado em offset " + std::to_string(offset) + " (declarado=" + std::to_string(length) +
                     " bytes)."};

    ParsedField lengthField;
    lengthField.identifier = "length";
    lengthField.name = "Length";
    lengthField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset + TAG_SIZE, LENGTH_SIZE);

    ParsedField valueField;
    valueField.identifier = "value";
    valueField.name = "Value";
    valueField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset + TAG_SIZE + LENGTH_SIZE, length);

    ParsedField field;
    field.identifier = identifier;
    field.name = name;
    field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, TAG_SIZE);
    field.values.push_back(std::move(lengthField));
    field.values.push_back(std::move(valueField));

    endOffset = offset + TAG_SIZE + LENGTH_SIZE + length;
    return field;
}

auto CosemDataParser::parseSequence(const std::vector<uint8_t> &data, size_t offset, const char *identifier, const char *name, size_t &endOffset)
    -> std::variant<ParsedField, Error>
{
    constexpr size_t TAG_SIZE = 1;
    constexpr size_t COUNT_SIZE = 1;

    if (offset + TAG_SIZE + COUNT_SIZE > data.size())
        return Error{"Dados insuficientes para count de " + std::string(name) + " em offset " + std::to_string(offset) + "."};

    uint8_t count = data[offset + TAG_SIZE];
    size_t pos = offset + TAG_SIZE + COUNT_SIZE;

    ParsedField countField;
    countField.identifier = "count";
    countField.name = "Count";
    countField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset + TAG_SIZE, COUNT_SIZE);

    ParsedField field;
    field.identifier = identifier;
    field.name = name;
    field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, TAG_SIZE);
    field.values.push_back(std::move(countField));

    for (uint8_t i = 0; i < count; ++i)
    {
        size_t elemEnd = pos;
        auto elem = CosemDataParser::parse(data, pos, elemEnd);

        if (std::holds_alternative<Error>(elem))
            return std::get<Error>(elem);

        field.values.push_back(std::get<ParsedField>(elem));
        pos = elemEnd;
    }

    endOffset = pos;
    return field;
}

auto CosemDataParser::parse(const std::vector<uint8_t> &data, size_t offset, size_t &endOffset) -> std::variant<ParsedField, Error>
{
    if (offset >= data.size())
        return Error{"Fim inesperado do frame ao tentar ler tag COSEM Data em offset " + std::to_string(offset) + "."};

    uint8_t tag = data[offset];

    switch (tag)
    {
    case 0x00: {
        ParsedField f;
        f.identifier = "null-data";
        f.name = "Null";
        f.value_bytes = "";
        endOffset = offset + 1;
        return f;
    }
    case 0x01:
        return parseSequence(data, offset, "array", "Array", endOffset);

    case 0x02:
        return parseSequence(data, offset, "structure", "Structure", endOffset);

    case 0x03:
        return parseFixed(data, offset, "boolean", "Boolean", 1, endOffset);

    case 0x04:
        return parseLengthPrefixed(data, offset, "bit-string", "Bit-String", endOffset);

    case 0x05:
        return parseFixed(data, offset, "double-long", "Double-Long (INT32)", 4, endOffset);

    case 0x06:
        return parseFixed(data, offset, "double-long-unsigned", "Double-Long-Unsigned (UINT32)", 4, endOffset);

    case 0x09:
        return parseLengthPrefixed(data, offset, "octet-string", "Octet-String", endOffset);

    case 0x0A:
        return parseLengthPrefixed(data, offset, "visible-string", "Visible-String", endOffset);

    case 0x0C:
        return parseLengthPrefixed(data, offset, "utf8-string", "UTF8-String", endOffset);

    case 0x0D:
        return parseFixed(data, offset, "bcd", "BCD (INT8)", 1, endOffset);

    case 0x0F:
        return parseFixed(data, offset, "integer", "Integer (INT8)", 1, endOffset);

    case 0x10:
        return parseFixed(data, offset, "long", "Long (INT16)", 2, endOffset);

    case 0x11:
        return parseFixed(data, offset, "unsigned", "Unsigned (UINT8)", 1, endOffset);

    case 0x12:
        return parseFixed(data, offset, "long-unsigned", "Long-Unsigned (UINT16)", 2, endOffset);

    case 0x14:
        return parseFixed(data, offset, "long64", "Long64 (INT64)", 8, endOffset);

    case 0x15:
        return parseFixed(data, offset, "long64-unsigned", "Long64-Unsigned (UINT64)", 8, endOffset);

    case 0x16:
        return parseFixed(data, offset, "enum", "Enum (UINT8)", 1, endOffset);

    case 0x17:
        return parseFixed(data, offset, "float32", "Float32", 4, endOffset);

    case 0x18:
        return parseFixed(data, offset, "float64", "Float64", 8, endOffset);

    case 0x19:
        return parseFixed(data, offset, "date-time", "Date-Time", 12, endOffset);

    case 0x1A:
        return parseFixed(data, offset, "date", "Date", 5, endOffset);

    case 0x1B:
        return parseFixed(data, offset, "time", "Time", 4, endOffset);

    default:
        return Error{"Tag COSEM Data desconhecida: 0x" +
                     [tag] {
                         std::string s(2, '0');
                         const char *h = "0123456789ABCDEF";
                         s[0] = h[tag >> 4];
                         s[1] = h[tag & 0xF];
                         return s;
                     }() +
                     " em offset " + std::to_string(offset) + "."};
    }
}
