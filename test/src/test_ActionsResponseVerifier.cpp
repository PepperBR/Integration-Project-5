#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/ACTIONS/RESPONSE/ActionResponseParser.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}
TEST_CASE("ActionResponseParser - Frame curto demais", "[ActionResponse]")
{
    auto data = hexToBytes("C7 01");
    auto r = ActionResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("ActionResponseParser - Subtipo desconhecido", "[ActionResponse]")
{
    auto data = hexToBytes("C7 09 41");
    auto r = ActionResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("ActionResponseParser - Subtipo não implementado (0x03)", "[ActionResponse]")
{
    auto data = hexToBytes("C7 03 41 00");
    auto r = ActionResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-NORMAL", "[ActionResponse]")
{
    SECTION("OK: result = success (0x00) sem return-params")
    {
        // sub=01 invoke=41 result=00 return-params=00
        auto data = hexToBytes("C7 01 41 00 00");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "action-response-normal");
    }

    SECTION("OK: result = hardware fault (0x01)")
    {
        auto data = hexToBytes("C7 01 41 01 00");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: resultado desconhecido")
    {
        auto data = hexToBytes("C7 01 41 55 00");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: frame incompleto — sem byte de resultado")
    {
        auto data = hexToBytes("C7 01 41");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-WITH-PBLOCK", "[ActionResponse]")
{
    SECTION("Erro: frame incompleto para pblock")
    {
        auto data = hexToBytes("C7 02 41 00 00 00");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-NEXT-PBLOCK", "[ActionResponse]")
{
    SECTION("OK: número de bloco 1")
    {
        // sub=04 invoke=41 block=00000001
        auto data = hexToBytes("C7 04 41 00 00 00 01");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame incompleto para block-number")
    {
        auto data = hexToBytes("C7 04 41 00 00 00");
        auto r = ActionResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}
