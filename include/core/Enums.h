#pragma once
#include <iomanip>

enum class Enums
{
    UNKNOWN,
    GET,
    SET,
    ACTION
};

enum class ServiceType : uint8_t
{
    NORMAL = 0x01,
    WITH_DATABLOCK = 0x02,
    WITH_LIST = 0x03
};
