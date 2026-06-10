#pragma once

#include "core/CommonVerifierTypes.h"
#include "core/enums.h"

#include <cstdint>
#include <variant>
#include <vector>

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
    static auto parse_header(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>;
};