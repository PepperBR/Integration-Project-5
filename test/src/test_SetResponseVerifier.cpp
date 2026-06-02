#include "core/CommandTypes/SET/SET_RESPONSE_VERIFIER.h"
#include "hexToBytes.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("SET_RESPONSE_VERIFIER Validation Tests")
{
    SET_RESPONSE_VERIFIER verifier;

    SECTION("Global Edge Cases")
    {
        std::string frame_too_short = "C5 01";
        auto result = verifier.verify(hexToBytes(frame_too_short));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        std::string unknown_subtype = "C5 99 81";
        result = verifier.verify(hexToBytes(unknown_subtype));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("SET-Response-Normal (0x01)")
    {
        // Missing ActionResult byte
        std::string action_result_missing = "C5 01 81";
        auto result = verifier.verify(hexToBytes(action_result_missing));
        REQUIRE_FALSE(result.valid);

        std::string valid_success = "C5 01 81 00";
        result = verifier.verify(hexToBytes(valid_success));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // 3-byte header + 0x03
        std::string valid_error = "C5 01 81 03";
        result = verifier.verify(hexToBytes(valid_error));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Extra tracking garbage payload bytes trailing behind the normal response
        std::string normal_with_extra_bytes = "C5 01 81 00 AA BB";
        result = verifier.verify(hexToBytes(normal_with_extra_bytes));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("SET-Response-with-DataBlock (0x02)")
    {
        // Truncated structure check
        std::string truncated_datablock = "C5 02 81 00 00";
        auto result = verifier.verify(hexToBytes(truncated_datablock));
        REQUIRE_FALSE(result.valid);

        // Exactly 7 bytes total
        std::string valid_datablock = "C5 02 81 00 00 00 01";
        result = verifier.verify(hexToBytes(valid_datablock));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());
        REQUIRE(result.fields.size() >= 2);

        // Extra tracking bytes trailing behind a datablock confirmation
        std::string datablock_with_garbage = "C5 02 81 00 00 00 01 FF EE";
        result = verifier.verify(hexToBytes(datablock_with_garbage));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }

    SECTION("SET-Response-with-List (0x03)")
    {
        // Count index parameter missing
        std::string count_missing = "C5 03 81";
        auto result = verifier.verify(hexToBytes(count_missing));
        REQUIRE_FALSE(result.valid);

        std::string valid_list = "C5 03 81 02 00 03";
        result = verifier.verify(hexToBytes(valid_list));
        REQUIRE(result.valid);
        REQUIRE(result.errors.empty());

        // Array structural breakdown truncation
        std::string truncated_list = "C5 03 81 02 00";
        result = verifier.verify(hexToBytes(truncated_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());

        // Extra tracking garbage/redundant payload bytes trailing behind the list array end
        std::string trailing_garbage_list = "C5 03 81 01 00 FF EE";
        result = verifier.verify(hexToBytes(trailing_garbage_list));
        REQUIRE_FALSE(result.valid);
        REQUIRE_FALSE(result.errors.empty());
    }
}