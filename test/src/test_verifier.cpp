#include "core/verifier.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("Verifier: Validating correct data", "[verifier]")
{
    Verifier verifier;
    std::string frame = "C4 01 41 00 09 02 41 31";

    auto result = verifier.validateData(frame);

    REQUIRE(result.valid == true);
    REQUIRE(result.fields.size() == 0);
    REQUIRE(result.errors.size() == 0);
}