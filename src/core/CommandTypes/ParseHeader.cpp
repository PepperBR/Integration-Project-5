#include "core/CommandTypes/ParseHeader.h"
#include <string>

auto ParseHeader::parse_header(const ServiceType service_type, const Priority priority, const ServiceClass service_class)
    -> std::variant<ParsedField, ValidationError>
{
    if (verify_subType(service_type) && verify_Priority(priority) && verify_ServiceClass(service_class))
    {
        return ParsedField{"Header", 0, 3, "Valid Header",
                           "Cabeçalho APDU válido. Tipo de Serviço: " + std::to_string(static_cast<int>(service_type)) +
                               ", Prioridade: " + std::to_string(static_cast<int>(priority)) +
                               ", Classe de Serviço: " + std::to_string(static_cast<int>(service_class))};
    }
    else
    {
        return ValidationError{0, "Cabeçalho APDU inválido. Verifique os campos de tipo de comando, subtipo, prioridade e classe de serviço.", ""};
    }
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