#include "core/CommandTypes/GET/GET_RESPONSE_VERIFIER.h"
#include "hexToBytes.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("GET_RESPONSE_VERIFIER Validation Tests")
{
    GET_RESPONSE_VERIFIER verifier;

    SECTION("Global Edge Cases")
    {
        std::string frame_too_short = "C0 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string unknown_subtype = "C0 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("GET-Response-Normal (0x01)")
    {
        // Missing Choice byte
        std::string choice_missing = "C0 01 81";
        auto result = verifier.verify(hexToBytes(choice_missing));
        REQUIRE_FALSE(result.valid);

        // Invalid choice byte value
        std::string invalid_choice = "C0 01 81 05";
        result = verifier.verify(hexToBytes(invalid_choice));
        REQUIRE_FALSE(result.valid);

        std::string valid_success = "C0 01 81 00 06";
        result = verifier.verify(hexToBytes(valid_success));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Choice 0x00 missing the data payload
        std::string success_missing_data = "C0 01 81 00";
        result = verifier.verify(hexToBytes(success_missing_data));
        REQUIRE_FALSE(result.valid);

        // Choice 0x01 with 1 byte Data-Access-Result
        std::string valid_error = "C0 01 81 01 03";
        result = verifier.verify(hexToBytes(valid_error));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Choice 0x01 but has extra trailing bytes
        std::string error_with_extra_bytes = "C0 01 81 01 03 AA BB";
        result = verifier.verify(hexToBytes(error_with_extra_bytes));
        REQUIRE_FALSE(result.valid);
    }

    SECTION("GET-Response-with-DataBlock (0x02)")
    {
        // Truncated structure check
        std::string truncated_datablock = "C0 02 81 01 00 00 00 05 00";
        auto result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);

        std::string valid_datablock_success = "C0 02 81 01 00 00 00 01 00 00 02 AA BB";
        result = verifier.verify(hexToBytes(valid_datablock_success));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Data block payload length mismatch
        std::string datablock_length_mismatch = "C0 02 81 01 00 00 00 01 00 00 02 AA";
        result = verifier.verify(hexToBytes(datablock_length_mismatch));
        REQUIRE_FALSE(result.valid);

        // Choice 0x01 (Block Error Code), Raw Length must declare 00 00
        std::string valid_datablock_error = "C0 02 81 01 00 00 00 01 01 00 00";
        result = verifier.verify(hexToBytes(valid_datablock_error));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Choice 0x01 but declaring a non-zero payload length
        std::string datablock_error_invalid_length = "C0 02 81 01 00 00 00 01 01 00 04";
        result = verifier.verify(hexToBytes(datablock_error_invalid_length));
        REQUIRE_FALSE(result.valid);

        // Invalid choice type flag inside the block header
        std::string datablock_invalid_choice = "C0 02 81 01 00 00 00 01 09 00 00";
        result = verifier.verify(hexToBytes(datablock_invalid_choice));
        REQUIRE_FALSE(result.valid);
    }

    SECTION("GET-Response-with-List (0x03)")
    {
        // Count index parameter missing
        std::string count_missing = "C0 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);

        std::string valid_list = "C0 03 81 02 "
                                 "00 05 "
                                 "01 03";
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Array structural breakdown truncation
        std::string truncated_list = "C0 03 81 02 00 05";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);

        // Item 1 declares success, but data payload byte is missing
        std::string success_item_missing_payload = "C0 03 81 01 00";
        result = verifier.verify(hexToBytes(success_item_missing_payload));
        REQUIRE_FALSE(result.valid);

        // Item 1 declares an access error, but error type code byte is missing
        std::string error_item_missing_payload = "C0 03 81 01 01";
        result = verifier.verify(hexToBytes(error_item_missing_payload));
        REQUIRE_FALSE(result.valid);

        // Row choice verification anomaly
        std::string list_invalid_item_choice = "C0 03 81 01 02 00";
        result = verifier.verify(hexToBytes(list_invalid_item_choice));
        REQUIRE_FALSE(result.valid);

        // Extra tracking garbage/redundant payload bytes trailing behind the list array end
        std::string trailing_garbage_list = "C0 03 81 01 00 05 FF EE";
        result = verifier.verify(hexToBytes(trailing_garbage_list));
        REQUIRE_FALSE(result.valid);
    }
}