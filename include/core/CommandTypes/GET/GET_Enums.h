#pragma once
#include <cstdint>

enum class GetServiceType : uint8_t
{
    NORMAL = 0x01,
    WITH_DATABLOCK = 0x02,
    WITH_LIST = 0x03
};

enum class GetDataResultChoice : uint8_t
{
    DATA_SUCCESS = 0x00,
    DATA_ACCESS_ERROR = 0x01
};