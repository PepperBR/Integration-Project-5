#include "core/utils/CosemDescriptorParser.h"
#include "core/utils/DlmsFrameUtils.h"

#include <string>

auto CosemDescriptorParser::parseDescriptor(const std::vector<uint8_t> &data, size_t offset, const char *idIdentifier, const char *idName,
                                            const char *outerIdentifier, const char *outerName) -> std::variant<ParsedField, Error>
{
    if (offset + DESCRIPTOR_SIZE > data.size())
        return Error{"Dados insuficientes para " + std::string(outerName) + "."};

    const size_t classIdAbs = offset + CLASS_ID_REL_OFFSET;
    const size_t obisAbs = offset + OBIS_REL_OFFSET;
    const size_t idAbs = offset + ID_REL_OFFSET;

    uint16_t classId = (static_cast<uint16_t>(data[classIdAbs]) << 8) | data[classIdAbs + 1];

    std::string obis = std::to_string(data[obisAbs]) + "." + std::to_string(data[obisAbs + 1]) + "." + std::to_string(data[obisAbs + 2]) + "." +
                       std::to_string(data[obisAbs + 3]) + "." + std::to_string(data[obisAbs + 4]) + "." + std::to_string(data[obisAbs + 5]);

    ParsedField classIdField;
    classIdField.identifier = "class-id";
    classIdField.name = "Class-Id";
    classIdField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, classIdAbs, CLASS_ID_SIZE);

    ParsedField obisField;
    obisField.identifier = "instance-id";
    obisField.name = "OBIS";
    obisField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, obisAbs, OBIS_SIZE);

    ParsedField idField;
    idField.identifier = idIdentifier;
    idField.name = idName;
    idField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, idAbs, ID_FIELD_SIZE);

    ParsedField field;
    field.identifier = outerIdentifier;
    field.name = outerName;
    field.value_bytes = DlmsFrameUtils::bytes_to_hex(data, offset, DESCRIPTOR_SIZE);
    field.values.push_back(std::move(classIdField));
    field.values.push_back(std::move(obisField));
    field.values.push_back(std::move(idField));
    return field;
}

auto CosemDescriptorParser::parseCosemAttributeDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return parseDescriptor(data, offset, "attribute-id", "Attribute-Id", "cosem-attribute-descriptor", "Cosem-Attribute-Descriptor");
}

auto CosemDescriptorParser::parseCosemMethodDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return parseDescriptor(data, offset, "method-id", "Method-Id", "cosem-method-descriptor", "Cosem-Method-Descriptor");
}
