#include "core/CommandTypes/GET/GET_REQUEST_VERIFIER.h"
#include "hexToBytes.h" // Ensures access to hex decoding utility helper

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("GET_REQUEST_VERIFIER Validation Tests")
{
    GET_REQUEST_VERIFIER verifier;

    SECTION("Global Edge Cases")
    {
        // Absolute minimum header check failure
        std::string frame_too_short = "C0 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Unknown Subtype variation check
        std::string unknown_subtype = "C0 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }

    SECTION("GET-Request-Normal (0x01)")
    {
        // Valid case: 3-byte header + 9-byte descriptor = 12 bytes
        std::string valid_normal = "C0 01 81 00 01 00 00 01 00 00 FF 02";
        auto result = verifier.verify(hexToBytes(valid_normal));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.size() >= 2); // Invoke ID and descriptor metadata parsed

        // Valid case with trailing access parameters (Selective access data allowed)
        std::string valid_with_access = "C0 01 81 00 01 00 00 01 00 00 FF 02 01 02 03";
        result = verifier.verify(hexToBytes(valid_with_access));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid case: Missing attribute ID (Truncated descriptor)
        std::string truncated_normal = "C0 01 81 00 01 00 00 01 00 00 FF";
        result = verifier.verify(hexToBytes(truncated_normal));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }

    SECTION("GET-Request-with-DataBlock (0x02)")
    {
        // Valid case: 3-byte header + 4-byte block integer = 7 bytes total
        std::string valid_datablock = "C0 02 81 00 00 00 05"; // Requesting Block #5
        auto result = verifier.verify(hexToBytes(valid_datablock));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.back().value == "5");

        // Invalid case: Truncated Block integer data
        std::string truncated_datablock = "C0 02 81 00 00 00";
        result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);

        // Invalid case: Extra bytes on datablock wrapper
        std::string oversized_datablock = "C0 02 81 00 00 00 05 AA BB";
        result = verifier.verify(hexToBytes(oversized_datablock));
        REQUIRE_FALSE(result.valid);
    }

    SECTION("GET-Request-with-List (0x03)")
    {
        // Truncated list count parameter test
        std::string count_missing = "C0 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);

        // Valid case: Count = 2, requires 4 + (2 * 9) = 22 bytes
        std::string valid_list = "C0 03 81 02 "
                                 "00 01 00 00 01 00 00 FF 02 " // Item 1 descriptor
                                 "00 03 00 00 02 00 00 FF 01"; // Item 2 descriptor
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid case: Declared 2 items but missing second descriptor payload block
        std::string truncated_list = "C0 03 81 02 00 01 00 00 01 00 00 FF 02";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);

        // Invalid case: Trailing data context anomalies
        std::string trailing_list = "C0 03 81 01 00 01 00 00 01 00 00 FF 02 CC DD";
        result = verifier.verify(hexToBytes(trailing_list));
        REQUIRE_FALSE(result.valid);
    }
}