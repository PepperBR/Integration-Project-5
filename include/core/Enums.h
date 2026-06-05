#pragma once

enum class XDlmsDataType
{
    UNKNOWN,
    GET_REQUEST,
    GET_RESPONSE,
    SET_REQUEST,
    SET_RESPONSE,
    ACTION_REQUEST,
    ACTION_RESPONSE
};

enum class ServiceType : uint8_t
{
    NORMAL = 0x01,
    WITH_DATABLOCK = 0x02,
    WITH_LIST = 0x03
};

enum class Priority : uint8_t
{
    NORMAL = 0,
    HIGH = 1
};

enum class ServiceClass : uint8_t
{
    UNCONFIRMED = 0,
    CONFIRMED = 1
};
