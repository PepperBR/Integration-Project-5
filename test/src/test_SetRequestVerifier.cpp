#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/SET/REQUEST/SetRequestParser.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}

TEST_CASE("SetRequestParser - Frame curto demais", "[SetRequest]")
{
    auto data = hexToBytes("C1 01");
    auto r = SetRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("SetRequestParser - Subtipo desconhecido", "[SetRequest]")
{
    auto data = hexToBytes("C1 09 41");
    auto r = SetRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("SetRequestParser - Subtipos não implementados", "[SetRequest]")
{
    SECTION("SET-REQUEST-WITH-LIST (0x04)")
    {
        auto data = hexToBytes("C1 04 41 00");
        auto r = SetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("SET-REQUEST-WITH-LIST-AND-FIRST-DATA-BLOCK (0x05)")
    {
        auto data = hexToBytes("C1 05 41 00");
        auto r = SetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-NORMAL", "[SetRequest]")
{
    SECTION("OK: descriptor sem seleção + dado octet-string 1 byte")
    {
        // tag=C1 sub=01 invoke=41 | class=0001 obis=01000108 00FF attr=02 | sel=00 | type=09 len=01 val=AA
        auto data = hexToBytes("C1 01 41 00 01 01 00 01 08 00 FF 02 00 09 01 AA");
        auto r = SetRequestParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "set-request-normal");
    }

    SECTION("Erro: frame incompleto — sem dado após o descriptor")
    {
        auto data = hexToBytes("C1 01 41 00 01 01 00 01 08 00 FF 02");
        auto r = SetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-WITH-FIRST-DATABLOCK", "[SetRequest]")
{
    SECTION("Erro: frame incompleto para datablock")
    {
        auto data = hexToBytes("C1 02 41 00 01 01 00 01 08 00 FF 02");
        auto r = SetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}
