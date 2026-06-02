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
        // Absolute minimum header check failure
        std::string frame_too_short = "C0 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Unknown Subtype variation check
        std::string unknown_subtype = "C0 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("GET-Response-Normal (0x01)")
    {
        // Missing Choice byte completely (Only 3 bytes header provided)
        std::string choice_missing = "C0 01 81";
        auto result = verifier.verify(hexToBytes(choice_missing));
        REQUIRE_FALSE(result.valid);

        // Invalid choice byte value (Must be 0x00 or 0x01)
        std::string invalid_choice = "C0 01 81 05";
        result = verifier.verify(hexToBytes(invalid_choice));
        REQUIRE_FALSE(result.valid);

        // Valid Success Case: Choice 0x00 with data payload (e.g., 1 byte payload)
        std::string valid_success = "C0 01 81 00 06";
        result = verifier.verify(hexToBytes(valid_success));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid Success Case: Choice 0x00 but completely missing the data payload
        std::string success_missing_data = "C0 01 81 00";
        result = verifier.verify(hexToBytes(success_missing_data));
        REQUIRE_FALSE(result.valid);

        // Valid Error Case: Choice 0x01 with 1 byte Data-Access-Result (Exactly 5 bytes)
        std::string valid_error = "C0 01 81 01 03"; // 0x03 could represent Read-Write-Denied
        result = verifier.verify(hexToBytes(valid_error));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid Error Case: Choice 0x01 but has redundant/extra trailing bytes
        std::string error_with_extra_bytes = "C0 01 81 01 03 AA BB";
        result = verifier.verify(hexToBytes(error_with_extra_bytes));
        REQUIRE_FALSE(result.valid);
    }

    SECTION("GET-Response-with-DataBlock (0x02)")
    {
        // Truncated structure check (Fails to meet the expected 12 header control bytes)
        std::string truncated_datablock = "C0 02 81 01 00 00 00 05 00";
        auto result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);

        // Valid Success Case: Last Block (0x01), Block #1, Choice 0x00, Raw Length 2, Data bytes [AA, BB]
        // Header (11 bytes) + 2 data bytes = 14 bytes absolute expected size
        std::string valid_datablock_success = "C0 02 81 01 00 00 00 01 00 00 02 AA BB";
        result = verifier.verify(hexToBytes(valid_datablock_success));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid Success Case: Data block payload length mismatch (declared 2 bytes, only provided 1)
        std::string datablock_length_mismatch = "C0 02 81 01 00 00 00 01 00 00 02 AA";
        result = verifier.verify(hexToBytes(datablock_length_mismatch));
        REQUIRE_FALSE(result.valid);

        // Valid Error Case: Choice 0x01 (Block Error Code), Raw Length must declare 00 00 (Exactly 12 bytes total)
        std::string valid_datablock_error = "C0 02 81 01 00 00 00 01 01 00 00";
        result = verifier.verify(hexToBytes(valid_datablock_error));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid Error Case: Choice 0x01 but maliciously declaring a non-zero payload length
        std::string datablock_error_invalid_length = "C0 02 81 01 00 00 00 01 01 00 04";
        result = verifier.verify(hexToBytes(datablock_error_invalid_length));
        REQUIRE_FALSE(result.valid);

        // Invalid Control Case: Invalid choice type flag inside the block header
        std::string datablock_invalid_choice = "C0 02 81 01 00 00 00 01 09 00 00";
        result = verifier.verify(hexToBytes(datablock_invalid_choice));
        REQUIRE_FALSE(result.valid);
    }

    SECTION("GET-Response-with-List (0x03)")
    {
        // Truncated case: Count index parameter missing completely
        std::string count_missing = "C0 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);

        // Valid Case: List Count = 2 items
        // Item 1: Choice 0x00 (Success) + 1 byte data value -> offset moves by 2
        // Item 2: Choice 0x01 (Error) + 1 byte access error code -> offset moves by 2
        std::string valid_list = "C0 03 81 02 "
                                 "00 05 " // Item 1: Success, Data=0x05
                                 "01 03"; // Item 2: Error, Code=0x03
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Invalid Case: Array structural breakdown truncation (Declares 2 items, only 1 provided)
        std::string truncated_list = "C0 03 81 02 00 05";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);

        // Invalid Case: Item 1 declares success, but data payload byte is completely missing
        std::string success_item_missing_payload = "C0 03 81 01 00";
        result = verifier.verify(hexToBytes(success_item_missing_payload));
        REQUIRE_FALSE(result.valid);

        // Invalid Case: Item 1 declares an access error, but error type code byte is missing
        std::string error_item_missing_payload = "C0 03 81 01 01";
        result = verifier.verify(hexToBytes(error_item_missing_payload));
        REQUIRE_FALSE(result.valid);

        // Invalid Case: Row choice verification anomaly (0x02 is not a valid item result type)
        std::string list_invalid_item_choice = "C0 03 81 01 02 00";
        result = verifier.verify(hexToBytes(list_invalid_item_choice));
        REQUIRE_FALSE(result.valid);

        // Invalid Case: Extra tracking garbage/redundant payload bytes trailing behind the list array end
        std::string trailing_garbage_list = "C0 03 81 01 00 05 FF EE";
        result = verifier.verify(hexToBytes(trailing_garbage_list));
        REQUIRE_FALSE(result.valid);
    }
}