#pragma once

#include "core/CommonVerifierTypes.h"
#include "core/enums.h"

#include <variant>

struct InvokeIdAndPriority
{
    uint8_t invokeId;
    Priority priority;
    ServiceClass serviceClass;
};

class ParseHeader
{
private:
    static auto verify_subType(ServiceType subType) -> bool;
    static auto verify_Priority(Priority priority) -> bool;
    static auto verify_ServiceClass(ServiceClass serviceClass) -> bool;

public:
    static auto decodeInvokeField(uint8_t value) -> InvokeIdAndPriority;
    static auto parse_header(const ServiceType service_type, const Priority priority, const ServiceClass service_class)
        -> std::variant<ParsedField, ValidationError>;
};
