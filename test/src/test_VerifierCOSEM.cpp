#include "core/VerifierCOSEM.h"
#include "hexToBytes.h"

#include <catch2/catch_test_macros.hpp>

#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

TEST_CASE("verifyCOSEM, data frame smaller than 4, frame invalid")
{
    VerifierCOSEM verifier;
    std::string frame_invalid;

    SECTION("Frame smaller than 4 bytes")
    {
        frame_invalid = "C4 01 41";

        auto result = verifier.verifyCOSEM(hexToBytes(frame_invalid));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.fields.empty());
        REQUIRE_FALSE(result.errors.empty());
        REQUIRE(result.errors.front().message == "Frame muito curto para ser uma APDU DLMS válida.");
        REQUIRE(result.errors.front().found == "C4 01 41");
    }
    SECTION("Frame empty")
    {
        frame_invalid = "";

        auto result = verifier.verifyCOSEM(hexToBytes(frame_invalid));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.fields.empty());
        REQUIRE_FALSE(result.errors.empty());
        REQUIRE(result.errors.front().message == "Frame muito curto para ser uma APDU DLMS válida.");
        REQUIRE(result.errors.front().found == "");
    }
    SECTION("Frame with single space")
    {
        frame_invalid = " ";

        auto result = verifier.verifyCOSEM(hexToBytes(frame_invalid));
        REQUIRE_FALSE(result.valid);
        REQUIRE(result.fields.empty());
        REQUIRE_FALSE(result.errors.empty());
        REQUIRE(result.errors.front().message == "Frame muito curto para ser uma APDU DLMS válida.");
        REQUIRE(result.errors.front().found == "");
    }
}