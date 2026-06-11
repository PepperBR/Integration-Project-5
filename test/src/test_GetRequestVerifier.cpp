#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}

TEST_CASE("GetRequestParser - Frame curto demais", "[GetRequest]")
{
    auto data = hexToBytes("C0 01");
    auto r = GetRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("GetRequestParser - Subtipo desconhecido", "[GetRequest]")
{
    auto data = hexToBytes("C0 0A 41");
    auto r = GetRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("GetRequestParser - GET-REQUEST-WITH-LIST não implementado", "[GetRequest]")
{
    auto data = hexToBytes("C0 03 41");
    auto r = GetRequestParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("GetRequestParser - GET-REQUEST-NORMAL sem seleção de acesso", "[GetRequest]")
{
    SECTION("Frame válido: sel=00 sem bytes extras")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 00");
        auto r = GetRequestParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.values.size() >= 2);
        REQUIRE(r.fields.identifier == "get-request-normal");
    }

    SECTION("Erro: sel=00 mas com bytes extras no final")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 00 FF");
        auto r = GetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: frame incompleto — sem byte de seleção")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02");
        auto r = GetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: flag de seleção inválido (nem 0x00 nem 0x01)")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 FF");
        auto r = GetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: flag sel=01 mas sem dados de seleção seguintes")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 01");
        auto r = GetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("OK: flag sel=01 com byte de dado de seleção presente")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 01 AA");
        auto r = GetRequestParser::verify(data);
        REQUIRE(ok(r));
    }
}

TEST_CASE("GetRequestParser - GET-REQUEST-NEXT", "[GetRequest]")
{
    SECTION("Frame válido: número de bloco 1")
    {
        auto data = hexToBytes("C0 02 41 00 00 00 01");
        auto r = GetRequestParser::verify(data);
        REQUIRE(ok(r));
        auto &last = r.fields.values.back();
        REQUIRE(last.identifier == "block-number");
        REQUIRE(last.value_bytes == "00 00 00 01");
    }

    SECTION("Frame válido: número de bloco 255")
    {
        auto data = hexToBytes("C0 02 41 00 00 00 FF");
        auto r = GetRequestParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame menor que 7 bytes (payload do bloco incompleto)")
    {
        auto data = hexToBytes("C0 02 41 00 00 00");
        auto r = GetRequestParser::verify(data);
        REQUIRE(hasError(r));
    }
}
