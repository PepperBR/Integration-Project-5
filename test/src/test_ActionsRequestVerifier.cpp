#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}

TEST_CASE("ActionRequestParser - Frame curto demais", "[ActionRequest]")
{
    auto data = hexToBytes("C3 01");
    auto r = ActionRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("ActionRequestParser - Subtipo desconhecido", "[ActionRequest]")
{
    auto data = hexToBytes("C3 09 41");
    auto r = ActionRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("ActionRequestParser - Subtipos não implementados", "[ActionRequest]")
{
    SECTION("ACTION-REQUEST-LIST (0x03)")
    {
        auto data = hexToBytes("C3 03 41 00");
        auto r = ActionRequestParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("ACTION-REQUEST-WITH-LIST-AND-FIRST-PBLOCK (0x05)")
    {
        auto data = hexToBytes("C3 05 41 00");
        auto r = ActionRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-NORMAL", "[ActionRequest]")
{
    SECTION("OK: sem parâmetros opcionais (params=0x00)")
    {
        auto data = hexToBytes("C3 01 41 00 01 01 00 01 08 00 FF 01 00");
        auto r = ActionRequestParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "action-request-normal");
    }

    SECTION("Erro: frame incompleto — sem byte de parâmetros")
    {
        auto data = hexToBytes("C3 01 41 00 01 01 00 01 08 00 FF 01");
        auto r = ActionRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-NEXT-PBLOCK", "[ActionRequest]")
{
    SECTION("OK: número de bloco 1")
    {
        auto data = hexToBytes("C3 02 41 00 00 00 01");
        auto r = ActionRequestParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame incompleto para block-number")
    {
        auto data = hexToBytes("C3 02 41 00 00 00");
        auto r = ActionRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}
