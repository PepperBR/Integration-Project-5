#include "core/CommandTypes/SET/SET_REQUEST_VERIFIER.h"
#include "hexToBytes.h" // Ensures access to hex decoding utility helper

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("SET_REQUEST_VERIFIER Validation Tests")
{
    SET_REQUEST_VERIFIER verifier;

    SECTION("Global Edge Cases")
    {
        std::string frame_too_short = "C1 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string unknown_subtype = "C1 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }

    SECTION("SET-Request-Normal (0x01)")
    {
        std::string valid_normal = "C1 01 81 00 01 00 00 01 00 00 FF 02 05";
        auto result = verifier.verify(hexToBytes(valid_normal));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.size() >= 2);

        // Missing attribute ID
        std::string truncated_normal = "C1 01 81 00 01 00 00 01 00 00 FF";
        result = verifier.verify(hexToBytes(truncated_normal));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Has exactly 12 bytes
        std::string missing_payload = "C1 01 81 00 01 00 00 01 00 00 FF 02";
        result = verifier.verify(hexToBytes(missing_payload));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }

    SECTION("SET-Request-with-DataBlock (0x02)")
    {
        std::string valid_datablock = "C1 02 81 00 00 00 00 01 AA BB";
        auto result = verifier.verify(hexToBytes(valid_datablock));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Truncated control block header
        std::string truncated_control = "C1 02 81 00 00 00";
        result = verifier.verify(hexToBytes(truncated_control));
        REQUIRE_FALSE(result.valid);

        // Missing raw data payload
        std::string empty_payload_datablock = "C1 02 81 01 00 00 00 05";
        result = verifier.verify(hexToBytes(empty_payload_datablock));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("SET-Request-with-List (0x03)")
    {
        // Truncated list count parameter test
        std::string count_missing = "C1 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);

        std::string valid_list = "C1 03 81 01 "
                                 "00 01 00 00 01 00 00 FF 02 "
                                 "12 34";
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Declared 1 item but array is truncated inside the descriptor sequence
        std::string truncated_descriptors = "C1 03 81 01 00 01 00 00 01";
        result = verifier.verify(hexToBytes(truncated_descriptors));
        REQUIRE_FALSE(result.valid);

        // Contains full descriptors list but has no matching values payload sequence following it
        std::string missing_data_list = "C1 03 81 01 00 01 00 00 01 00 00 FF 02";
        result = verifier.verify(hexToBytes(missing_data_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.errors.size() != 0);
    }
}