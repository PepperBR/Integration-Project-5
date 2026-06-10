#include "core/CommandTypes/ParseHeader.h"
#include <iomanip>
#include <sstream>
#include <string>

static auto byte_to_hex(uint8_t b) -> std::string
{
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(b);
    return oss.str();
}

auto ParseHeader::parse_header(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    if (offset + 1 >= data.size())
        return Error{"Cabeçalho APDU incompleto: bytes de service-type ou invoke-id-and-priority ausentes."};

    auto serviceType = static_cast<ServiceType>(data[offset]);
    uint8_t invokeRaw = data[offset + 1];
    auto invokeInfo = decodeInvokeField(invokeRaw);

    if (!verify_subType(serviceType))
        return Error{"Cabeçalho APDU inválido. Verifique os campos de tipo de comando, subtipo, prioridade e classe de serviço."};
    if (!verify_Priority(invokeInfo.priority))
        return Error{"Cabeçalho APDU inválido. Verifique os campos de tipo de comando, subtipo, prioridade e classe de serviço."};
    if (!verify_ServiceClass(invokeInfo.serviceClass))
        return Error{"Cabeçalho APDU inválido. Verifique os campos de tipo de comando, subtipo, prioridade e classe de serviço."};

    ParsedField invokeIdField;
    invokeIdField.identifier = "invoke-id";
    invokeIdField.name = "Invoke-Id";
    invokeIdField.value_bytes = byte_to_hex(invokeInfo.invokeId);

    ParsedField priorityField;
    priorityField.identifier = "priority";
    priorityField.name = "Priority";
    priorityField.value_bytes = (invokeInfo.priority == Priority::HIGH) ? "HIGH" : "NORMAL";

    ParsedField serviceClassField;
    serviceClassField.identifier = "service-class";
    serviceClassField.name = "Service-Class";
    serviceClassField.value_bytes = (invokeInfo.serviceClass == ServiceClass::CONFIRMED) ? "CONFIRMED" : "UNCONFIRMED";

    ParsedField invokeAndPriorityField;
    invokeAndPriorityField.identifier = "invoke-id-and-priority";
    invokeAndPriorityField.name = "Invoke-Id-And-Priority";
    invokeAndPriorityField.value_bytes = byte_to_hex(invokeRaw);
    invokeAndPriorityField.values.push_back(std::move(invokeIdField));
    invokeAndPriorityField.values.push_back(std::move(priorityField));
    invokeAndPriorityField.values.push_back(std::move(serviceClassField));

    return invokeAndPriorityField;
}

auto ParseHeader::verify_subType(ServiceType subType) -> bool
{
    switch (static_cast<uint8_t>(subType))
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
        return true;
    default:
        return false;
    }
};

auto ParseHeader::verify_Priority(Priority priority) -> bool
{
    switch (priority)
    {
    case Priority::HIGH:
    case Priority::NORMAL:
        return true;
    default:
        return false;
    }
};

auto ParseHeader::verify_ServiceClass(ServiceClass serviceClass) -> bool
{
    switch (serviceClass)
    {
    case ServiceClass::CONFIRMED:
    case ServiceClass::UNCONFIRMED:
        return true;
    default:
        return false;
    }
};

auto ParseHeader::decodeInvokeField(uint8_t value) -> InvokeIdAndPriority
{
    return {static_cast<uint8_t>(value & 0x0F), (value & 0x80) ? Priority::HIGH : Priority::NORMAL,
            (value & 0x40) ? ServiceClass::CONFIRMED : ServiceClass::UNCONFIRMED};
}
