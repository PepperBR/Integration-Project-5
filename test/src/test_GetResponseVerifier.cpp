#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/GET/RESPONSE/GetResponseParser.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}

TEST_CASE("GetResponseParser - Frame curto demais", "[GetResponse]")
{
    auto data = hexToBytes("C4 01");
    auto r = GetResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("GetResponseParser - Subtipo desconhecido", "[GetResponse]")
{
    auto data = hexToBytes("C4 0F 41");
    auto r = GetResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("GetResponseParser - GET-RESPONSE-WITH-LIST não implementado", "[GetResponse]")
{
    auto data = hexToBytes("C4 03 41");
    auto r = GetResponseParser::verify(data);
    REQUIRE(hasError(r));
}

TEST_CASE("GetResponseParser - GET-RESPONSE-NORMAL", "[GetResponse]")
{
    SECTION("OK: Data-Access-Result = success (choice=01, enum=00)")
    {
        auto data = hexToBytes("C4 01 41 01 00");
        auto r = GetResponseParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "get-response-normal");
    }

    SECTION("OK: Data-Access-Result = hardware fault (choice=01, enum=01)")
    {
        auto data = hexToBytes("C4 01 41 01 01");
        auto r = GetResponseParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame incompleto sem byte de choice")
    {
        auto data = hexToBytes("C4 01 41");
        auto r = GetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: choice=01 sem byte de enum subsequente")
    {
        auto data = hexToBytes("C4 01 41 01");
        auto r = GetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }

    SECTION("Erro: tag de choice inválida")
    {
        auto data = hexToBytes("C4 01 41 05 00");
        auto r = GetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}

TEST_CASE("GetResponseParser - GET-RESPONSE-WITH-DATABLOCK", "[GetResponse]")
{
    SECTION("OK: último bloco com dados")
    {
        auto data = hexToBytes("C4 02 41 01 00 00 00 01 00 AA BB");
        auto r = GetResponseParser::verify(data);
        REQUIRE(ok(r));
        REQUIRE(r.fields.identifier == "get-response-with-datablock");
    }

    SECTION("OK: bloco intermediário")
    {
        auto data = hexToBytes("C4 02 41 00 00 00 00 01 00 11 22 33");
        auto r = GetResponseParser::verify(data);
        REQUIRE(ok(r));
    }

    SECTION("Erro: frame menor que o mínimo estrutural")
    {
        auto data = hexToBytes("C4 02 41 01 00 00 00 01");
        auto r = GetResponseParser::verify(data);
        REQUIRE(hasError(r));
    }
}
