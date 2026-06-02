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
        std::string frame_too_short = "C0 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string unknown_subtype = "C0 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }

    SECTION("GET-Request-Normal (0x01)")
    {
        std::string valid_normal = "C0 01 81 00 01 00 00 01 00 00 FF 02";
        auto result = verifier.verify(hexToBytes(valid_normal));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.size() >= 2);

        std::string valid_with_access = "C0 01 81 00 01 00 00 01 00 00 FF 02 01 02 03";
        result = verifier.verify(hexToBytes(valid_with_access));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Missing attribute ID
        std::string truncated_normal = "C0 01 81 00 01 00 00 01 00 00 FF";
        result = verifier.verify(hexToBytes(truncated_normal));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }

    SECTION("GET-Request-with-DataBlock (0x02)")
    {
        std::string valid_datablock = "C0 02 81 00 00 00 05";
        auto result = verifier.verify(hexToBytes(valid_datablock));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.back().value == "5");

        // Truncated Block integer data
        std::string truncated_datablock = "C0 02 81 00 00 00";
        result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);

        // Extra bytes on datablock wrapper
        std::string oversized_datablock = "C0 02 81 00 00 00 05 AA BB";
        result = verifier.verify(hexToBytes(oversized_datablock));
        REQUIRE_FALSE(result.valid);
    }

    SECTION("GET-Request-with-List (0x03)")
    {
        // Truncated list count parameter
        std::string count_missing = "C0 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);

        std::string valid_list = "C0 03 81 02 "
                                 "00 01 00 00 01 00 00 FF 02 "
                                 "00 03 00 00 02 00 00 FF 01";
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Declared 2 items but missing second descriptor payload
        std::string truncated_list = "C0 03 81 02 00 01 00 00 01 00 00 FF 02";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);

        // Trailing data context anomalies
        std::string trailing_list = "C0 03 81 01 00 01 00 00 01 00 00 FF 02 CC DD";
        result = verifier.verify(hexToBytes(trailing_list));
        REQUIRE_FALSE(result.valid);
    }
}