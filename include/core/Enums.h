#pragma once

#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// Top-level APDU discriminators (byte[0] of an xDLMS frame)
// ─────────────────────────────────────────────────────────────────────────────
enum class XDlmsApduTag : uint8_t
{
    // ── No ciphering (SN referencing) ────────────────────────────────────────
    INITIATE_REQUEST = 0x01,
    READ_REQUEST = 0x05,
    WRITE_REQUEST = 0x06,
    INITIATE_RESPONSE = 0x08,
    READ_RESPONSE = 0x0C,
    WRITE_RESPONSE = 0x0D,
    CONFIRMED_SERVICE_ERROR = 0x0E,
    DATA_NOTIFICATION = 0x0F,
    UNCONFIRMED_WRITE_REQUEST = 0x16,
    INFORMATION_REPORT_REQUEST = 0x18,

    // ── No ciphering (LN referencing) ────────────────────────────────────────
    GET_REQUEST = 0xC0,                // 192
    SET_REQUEST = 0xC1,                // 193
    EVENT_NOTIFICATION_REQUEST = 0xC2, // 194
    ACTION_REQUEST = 0xC3,             // 195
    GET_RESPONSE = 0xC4,               // 196
    SET_RESPONSE = 0xC5,               // 197
    ACTION_RESPONSE = 0xC7,            // 199

    // ── Global ciphering (LN) ─────────────────────────────────────────────────
    GLO_GET_REQUEST = 0xC8,
    GLO_SET_REQUEST = 0xC9,
    GLO_EVENT_NOTIFICATION = 0xCA,
    GLO_ACTION_REQUEST = 0xCB,
    GLO_GET_RESPONSE = 0xCC,
    GLO_SET_RESPONSE = 0xCD,
    GLO_ACTION_RESPONSE = 0xCF,

    // ── Dedicated ciphering (LN) ──────────────────────────────────────────────
    DED_GET_REQUEST = 0xD0,
    DED_SET_REQUEST = 0xD1,
    DED_EVENT_NOTIFICATION = 0xD2,
    DED_ACTION_REQUEST = 0xD3,
    DED_GET_RESPONSE = 0xD4,
    DED_SET_RESPONSE = 0xD5,
    DED_ACTION_RESPONSE = 0xD7,

    // ── General APDUs ─────────────────────────────────────────────────────────
    EXCEPTION_RESPONSE = 0xD8,     // 216
    ACCESS_REQUEST = 0xD9,         // 217
    ACCESS_RESPONSE = 0xDA,        // 218
    GENERAL_GLO_CIPHERING = 0xDB,  // 219
    GENERAL_DED_CIPHERING = 0xDC,  // 220
    GENERAL_CIPHERING = 0xDD,      // 221
    GENERAL_SIGNING = 0xDF,        // 223
    GENERAL_BLOCK_TRANSFER = 0xE0, // 224
};

// ─────────────────────────────────────────────────────────────────────────────
// Service sub-type (byte[1] — choice tag inside each command family)
// ─────────────────────────────────────────────────────────────────────────────
enum class ServiceType : uint8_t
{
    NORMAL = 0x01,
    WITH_DATABLOCK = 0x02,
    WITH_LIST = 0x03,
    WITH_FIRST_DATABLOCK = 0x02, // alias for SET-request
    WITH_LIST_AND_FIRST_DATABLOCK = 0x05,
    LAST_DATABLOCK = 0x03,
    LAST_DATABLOCK_WITH_LIST = 0x04,
    // Action sub-types
    NEXT_PBLOCK = 0x02,
    WITH_FIRST_PBLOCK = 0x04,
    WITH_LIST_AND_FIRST_PBLOCK = 0x05,
    WITH_PBLOCK = 0x06,
    WITH_OPTIONAL_DATA = 0x01, // action-response-normal
    WITH_PBLOCK_RESPONSE = 0x02,
    WITH_LIST_RESPONSE = 0x03,
    NEXT_PBLOCK_RESPONSE = 0x04,
};

// ─────────────────────────────────────────────────────────────────────────────
// Invoke-Id-And-Priority bit-field sub-values
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
// Data-Access-Result (used by GET response and SET response)
// ─────────────────────────────────────────────────────────────────────────────
enum class DataAccessResult : uint8_t
{
    SUCCESS = 0,
    HARDWARE_FAULT = 1,
    TEMPORARY_FAILURE = 2,
    READ_WRITE_DENIED = 3,
    OBJECT_UNDEFINED = 4,
    OBJECT_CLASS_INCONSISTENT = 9,
    OBJECT_UNAVAILABLE = 11,
    TYPE_UNMATCHED = 12,
    SCOPE_OF_ACCESS_VIOLATED = 13,
    DATA_BLOCK_UNAVAILABLE = 14,
    LONG_GET_ABORTED = 15,
    NO_LONG_GET_IN_PROGRESS = 16,
    LONG_SET_ABORTED = 17,
    NO_LONG_SET_IN_PROGRESS = 18,
    DATA_BLOCK_NUMBER_INVALID = 19,
    OTHER_REASON = 250
};

// ─────────────────────────────────────────────────────────────────────────────
// Action-Result
// ─────────────────────────────────────────────────────────────────────────────
enum class ActionResult : uint8_t
{
    SUCCESS = 0,
    HARDWARE_FAULT = 1,
    TEMPORARY_FAILURE = 2,
    READ_WRITE_DENIED = 3,
    OBJECT_UNDEFINED = 4,
    OBJECT_CLASS_INCONSISTENT = 9,
    OBJECT_UNAVAILABLE = 11,
    TYPE_UNMATCHED = 12,
    SCOPE_OF_ACCESS_VIOLATED = 13,
    DATA_BLOCK_UNAVAILABLE = 14,
    LONG_ACTION_ABORTED = 15,
    NO_LONG_ACTION_IN_PROGRESS = 16,
    OTHER_REASON = 250
};

// ─────────────────────────────────────────────────────────────────────────────
// ExceptionResponse state-error
// ─────────────────────────────────────────────────────────────────────────────
enum class ExceptionStateError : uint8_t
{
    SERVICE_NOT_ALLOWED = 1,
    SERVICE_UNKNOWN = 2
};

// ─────────────────────────────────────────────────────────────────────────────
// ExceptionResponse service-error choice tag
// ─────────────────────────────────────────────────────────────────────────────
enum class ExceptionServiceError : uint8_t
{
    OPERATION_NOT_POSSIBLE = 1,
    SERVICE_NOT_SUPPORTED = 2,
    OTHER_REASON = 3,
    PDU_TOO_LONG = 4,
    DECIPHERING_ERROR = 5,
    INVOCATION_COUNTER_ERROR = 6
};

// ─────────────────────────────────────────────────────────────────────────────
// High-level discriminator used by the dispatcher / UI layer
// ─────────────────────────────────────────────────────────────────────────────
enum class XDlmsDataType
{
    UNKNOWN,
    GET_REQUEST,
    GET_RESPONSE,
    SET_REQUEST,
    SET_RESPONSE,
    ACTION_REQUEST,
    ACTION_RESPONSE,
    EXCEPTION_RESPONSE,
    ACCESS_REQUEST,
    ACCESS_RESPONSE,
    EVENT_NOTIFICATION_REQUEST,
    DATA_NOTIFICATION,
    GENERAL_BLOCK_TRANSFER,
    GENERAL_CIPHERING,
    GENERAL_SIGNING,
};
