#include "core/verifier.h"

#include <catch2/catch_test_macros.hpp>

#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

struct TestCaseData
{
    std::string description;
    std::string frame;
    bool expected_valid;
};

static std::vector<uint8_t> hexToBytes(const std::string &hex)
{
    std::vector<uint8_t> bytes;

    std::string clean;

    for (char c : hex)
    {
        if (c != ' ' && c != '-')
        {
            clean += c;
        }
    }

    if (clean.size() % 2 != 0)
    {
        throw std::runtime_error("Quantidade ímpar de caracteres hex.");
    }

    for (size_t i = 0; i < clean.size(); i += 2)
    {
        if (!std::isxdigit(static_cast<unsigned char>(clean[i])) ||

            !std::isxdigit(static_cast<unsigned char>(clean[i + 1])))
        {
            throw std::runtime_error("Hexadecimal inválido.");
        }

        bytes.push_back(static_cast<uint8_t>(std::stoul(clean.substr(i, 2), nullptr, 16)));
    }

    return bytes;
}
TEST_CASE("Verifier: Validating correct DLMS frames", "[verifier]")
{
    Verifier verifier;

    std::vector<TestCaseData> test_cases = {
        {"Simple Octet String", "C4 01 41 00 09 02 41 31", true},
        {"Octet String Empty", "C4 01 41 00 09 00", true},
        {"Visible ASCII String", "C4 01 41 00 09 05 48 45 4C 4C 4F", true},
        {"UInt8 Minimum", "C4 01 41 00 11 00", true},
        {"UInt8 Maximum", "C4 01 41 00 11 FF", true},
        {"Simple Array", "C4 01 41 00 01 02 11 01 11 02", true},
        {"Large Array",
         "C4 01 41 00 "
         "01 05 "
         "11 01 "
         "11 02 "
         "11 03 "
         "11 04 "
         "11 05",
         true},
        {"Simple Structure", "C4 01 41 00 02 02 11 01 11 02", true},
        {"Nested Structure",
         "C4 01 41 00 "
         "02 02 "
         "11 01 "
         "02 02 11 02 11 03",
         true},
        {"Complex Frame with Multiple Data Types",
         "C4 01 41 00 "
         "01 01 "
         "02 03 "
         "09 02 53 31 "
         "09 0C FF FF 01 01 FF 00 00 00 FF FF FF FF "
         "09 02 57 31",
         true},
        {"Frame with Interleaved UInt8",
         "C4 01 41 00 "
         "01 01 02 08 "
         "09 02 57 31 "
         "11 01 11 01 11 01 11 01 "
         "11 01 11 01 11 01",
         true},
        {"Hyphen-separated Frame",
         "C4-01-41-00-01-03-02-0D-0A-00-0A-07-32-33-30-39-32-31-31-11-02-16-01-11-00-11-66-11-10-11-01-09-00-11-01-09-00-03-00-09-00", true},
        {"GET Response Normal", "C4 01 41 00 11 01", true},
        {"GET Response With DataBlock", "C4 02 41 00 11 01", true},
        {"GET Response With List", "C4 03 41 00 11 01", true}};

    for (const auto &tc : test_cases)
    {
        DYNAMIC_SECTION("Scenario: " << tc.description)
        {
            auto result = verifier.validateData(hexToBytes(tc.frame));
            REQUIRE(result.valid == tc.expected_valid);
            CHECK(result.errors.empty());
            CHECK_FALSE(result.fields.empty());
        }
    }
}

TEST_CASE("Verifier: Detecting invalid DLMS frames", "[verifier]")
{
    Verifier verifier;

    std::vector<TestCaseData> invalid_cases = {{"Invalid APDU Tag", "AA 01 41 00 09 02 41 31", false},
                                               {"Empty Frame", "", false},
                                               {"Too Short Frame", "C4 01", false},
                                               {"Only APDU Tag", "C4", false},
                                               {"Invalid Octet String Length", "C4 01 41 00 09 05 41 31", false},
                                               {"Octet String Missing Size", "C4 01 41 00 09", false},
                                               {"Octet String Truncated", "C4 01 41 00 09 03 41", false},
                                               {"Incomplete UInt8", "C4 01 41 00 11", false},
                                               {"Incomplete Array", "C4 01 41 00 01", false},
                                               {"Array Missing Elements", "C4 01 41 00 01 03 11 01", false},
                                               {"Incomplete Structure", "C4 01 41 00 02", false},
                                               {"Structure Missing Elements", "C4 01 41 00 02 02 11 01", false},
                                               {"Unknown DLMS Tag", "C4 01 41 00 AA BB CC", false},
                                               {"Unknown Payload Type", "C4 01 41 00 FE 00", false},
                                               {"Corrupted Payload", "C4 01 41 00 09 FF FF FF", false},
                                               {"Garbage Payload", "C4 01 41 00 FF FF FF FF", false},
                                               {"Invalid Response Type", "C4 FF 41 00 11 01", false},
                                               {"Missing Invoke ID", "C4 01", false},
                                               {"Missing Result Code", "C4 01 41", false}};

    for (const auto &tc : invalid_cases)
    {
        DYNAMIC_SECTION("Scenario: " << tc.description)
        {
            auto result = verifier.validateData(hexToBytes(tc.frame));
            REQUIRE_FALSE(result.valid);
            CHECK_FALSE(result.errors.empty());
        }
    }
}

TEST_CASE("Verifier: Invalid hexadecimal input", "[verifier]")
{
    REQUIRE_THROWS(hexToBytes("C4 01 GG"));
    REQUIRE_THROWS(hexToBytes("C4 01 4"));
    REQUIRE_THROWS(hexToBytes("ZZ"));
    REQUIRE_THROWS(hexToBytes("##"));
    REQUIRE_THROWS(hexToBytes("C4 01 HH"));
    REQUIRE_THROWS(hexToBytes("XY"));
    REQUIRE_THROWS(hexToBytes("123"));
    REQUIRE_THROWS(hexToBytes("C4-01-XX-00"));
}

TEST_CASE("Verifier: Different frame formatting", "[verifier]")
{
    Verifier verifier;

    SECTION("Space separated")
    {
        auto result = verifier.validateData(hexToBytes("C4 01 41 00 11 01"));
        REQUIRE(result.valid);
    }

    SECTION("Hyphen separated")
    {
        auto result = verifier.validateData(hexToBytes("C4-01-41-00-11-01"));
        REQUIRE(result.valid);
    }

    SECTION("Mixed formatting")
    {
        auto result = verifier.validateData(hexToBytes("C4 01-41 00-11 01"));
        REQUIRE(result.valid);
    }

    SECTION("Lowercase hex")
    {
        auto result = verifier.validateData(hexToBytes("c4 01 41 00 11 ff"));
        REQUIRE(result.valid);
    }
}
