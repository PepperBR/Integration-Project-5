#include "core/CommandTypes/ACTIONS/ACTIONS_RESPONSE_VERIFIER.h"
#include "hexToBytes.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("ACTIONS_RESPONSE_VERIFIER Validation Tests")
{
    ACTIONS_RESPONSE_VERIFIER verifier;

    SECTION("Global Edge Cases")
    {
        std::string frame_too_short = "C7 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string unknown_subtype = "C7 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("ACTION-Response-Normal (0x01)")
    {
        // Missing ActionResult byte
        std::string result_missing = "C7 01 81";
        auto result = verifier.verify(hexToBytes(result_missing));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string valid_success_empty = "C7 01 81 00";
        result = verifier.verify(hexToBytes(valid_success_empty));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        std::string valid_success_with_params = "C7 01 81 00 01 02 03";
        result = verifier.verify(hexToBytes(valid_success_with_params));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        std::string valid_error_state = "C7 01 81 03";
        result = verifier.verify(hexToBytes(valid_error_state));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Indicates failure but has redundant trailing data
        std::string error_with_trailing_garbage = "C7 01 81 03 AA BB";
        result = verifier.verify(hexToBytes(error_with_trailing_garbage));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("ACTION-Response-with-DataBlock (0x02)")
    {
        // Truncated structure
        std::string truncated_datablock = "C7 02 81 01 00 00 00 05 00";
        auto result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string valid_datablock = "C7 02 81 01 00 00 00 01 00 02 AA BB";
        result = verifier.verify(hexToBytes(valid_datablock));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Data block payload length mismatch
        std::string datablock_length_short = "C7 02 81 01 00 00 00 01 00 02 AA";
        result = verifier.verify(hexToBytes(datablock_length_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Data block payload length mismatch
        std::string datablock_length_oversized = "C7 02 81 01 00 00 00 01 00 02 AA BB CC DD";
        result = verifier.verify(hexToBytes(datablock_length_oversized));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("ACTION-Response-with-List (0x03)")
    {
        // Count parameter missing
        std::string count_missing = "C7 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string valid_list = "C7 03 81 02 "
                                 "00 06 "
                                 "01 03";
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Structural truncation
        std::string truncated_list = "C7 03 81 02 00 06";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Mandatory error code byte is missing
        std::string error_item_missing_code = "C7 03 81 01 01";
        result = verifier.verify(hexToBytes(error_item_missing_code));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Redundant trailing payload
        std::string trailing_garbage_list = "C7 03 81 01 00 06 FF EE";
        result = verifier.verify(hexToBytes(trailing_garbage_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }
}