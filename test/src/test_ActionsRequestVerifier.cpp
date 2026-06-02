#include "core/CommandTypes/ACTIONS/ACTIONS_REQUEST_VERIFIER.h"
#include "hexToBytes.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("ACTION_REQUEST_VERIFIER Validation Tests")
{
    ACTIONS_REQUEST_VERIFIER verifier;

    SECTION("Global Edge Cases")
    {
        // Absolute minimum header check failure
        std::string frame_too_short = "C6 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Unknown Subtype variation check (Using ACTION Request command tag 0xC6)
        std::string unknown_subtype = "C6 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("ACTION-Request-Normal (0x01)")
    {
        std::string valid_normal = "C6 01 81 00 08 00 00 60 01 00 FF 01";
        auto result = verifier.verify(hexToBytes(valid_normal));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.size() >= 2);

        std::string valid_with_params = "C6 01 81 00 08 00 00 60 01 00 FF 01 02 01 0A";
        result = verifier.verify(hexToBytes(valid_with_params));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Missing method ID
        std::string truncated_normal = "C6 01 81 00 08 00 00 60 01 00 FF";
        result = verifier.verify(hexToBytes(truncated_normal));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("ACTION-Request-with-DataBlock (0x02)")
    {
        std::string valid_datablock = "C6 02 81 00 00 00 03";
        auto result = verifier.verify(hexToBytes(valid_datablock));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        std::string datablock_with_payload = "C6 02 81 00 00 00 03 AA BB CC DD";
        result = verifier.verify(hexToBytes(datablock_with_payload));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        std::string truncated_datablock = "C6 02 81 00 00 00";
        result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("ACTION-Request-with-List (0x03)")
    {
        std::string count_missing = "C6 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string valid_list = "C6 03 81 02 "
                                 "00 08 00 00 60 01 00 FF 01 "
                                 "00 0F 00 00 28 00 00 FF 02";
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        std::string list_with_trailing = "C6 03 81 01 "
                                         "00 08 00 00 60 01 00 FF 01 "
                                         "11 22 33";
        result = verifier.verify(hexToBytes(list_with_trailing));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Declared 2 items but missing second method
        std::string truncated_list = "C6 03 81 02 00 08 00 00 60 01 00 FF 01";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }
}