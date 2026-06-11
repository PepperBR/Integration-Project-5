#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/SET/RESPONSE/SetResponseParser.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}

TEST_CASE("SetResponseParser - Frame curto demais", "[SetResponse]")
{
    auto data = hexToBytes("C5 01");
    auto r = SetResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("SetResponseParser - Subtipo desconhecido", "[SetResponse]")
{
    auto data = hexToBytes("C5 09 41");
    auto r = SetResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("SetResponseParser - Subtipos não implementados", "[SetResponse]")
{
    SECTION("SET-RESPONSE-LAST-DATABLOCK-WITH-LIST (0x04)")
    {
        auto data = hexToBytes("C5 04 41 00");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("SET-RESPONSE-WITH-LIST (0x05)")
    {
        auto data = hexToBytes("C5 05 41 00");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-NORMAL", "[SetResponse]")
{
    SECTION("OK: result = success (0x00)")
    {
        auto data = hexToBytes("C5 01 41 00");
        auto r = SetResponseParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "set-response-normal");
    }

    SECTION("OK: result = hardware fault (0x01)")
    {
        auto data = hexToBytes("C5 01 41 01");
        auto r = SetResponseParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame incompleto — sem byte de resultado")
    {
        auto data = hexToBytes("C5 01 41");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: bytes extras após o resultado")
    {
        auto data = hexToBytes("C5 01 41 00 FF");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: código de resultado desconhecido")
    {
        auto data = hexToBytes("C5 01 41 55");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-DATABLOCK", "[SetResponse]")
{
    SECTION("OK: número de bloco 1")
    {
        // sub=02 invoke=41 block=00000001
        auto data = hexToBytes("C5 02 41 00 00 00 01");
        auto r = SetResponseParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "set-response-datablock");
    }

    SECTION("Erro: frame incompleto para block-number")
    {
        auto data = hexToBytes("C5 02 41 00 00 00");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-LAST-DATABLOCK", "[SetResponse]")
{
    SECTION("OK: resultado success + número de bloco 1")
    {
        // sub=03 invoke=41 result=00 block=00000001
        auto data = hexToBytes("C5 03 41 00 00 00 00 01");
        auto r = SetResponseParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame incompleto")
    {
        auto data = hexToBytes("C5 03 41 00 00 00");
        auto r = SetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}
