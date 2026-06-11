#include <catch2/catch_test_macros.hpp>

#include "core/XDlms.h"
#include "hexToBytes.h"

static bool hasError(const FrameResponse &r)
{
    return r.error.has_value();
}

static bool ok(const FrameResponse &r)
{
    return !r.error.has_value();
}

TEST_CASE("XDlms::decode - Frame inválido / tag desconhecida", "[XDlms]")
{
    SECTION("Frame vazio retorna erro de frame muito curto")
    {
        std::vector<uint8_t> data{};
        auto r = XDlms::decode(data);
        REQUIRE(hasError(r));
        REQUIRE(r.error->message.find("curto") != std::string::npos);
    }

    SECTION("Frame de apenas 3 bytes retorna erro de frame muito curto")
    {
        auto data = hexToBytes("C0 01 41");
        auto r = XDlms::decode(data);
        REQUIRE(hasError(r));
    }

    SECTION("Tag desconhecida retorna erro de tipo não suportado")
    {
        auto data = hexToBytes("AA 01 41 00");
        auto r = XDlms::decode(data);
        REQUIRE(hasError(r));
        REQUIRE(r.error->message.find("desconhecido") != std::string::npos);
    }

    SECTION("XDlms preenche identifier e name no campo raiz")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 00");
        auto r = XDlms::decode(data);
        REQUIRE(r.fields.identifier == "XDLMS-APDU::CHOICE");
        REQUIRE(r.fields.name == "XDLMS-APDU");
    }
}

TEST_CASE("XDlms::decode - Despacha corretamente para GET-REQUEST", "[XDlms]")
{
    auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 00");
    auto r = XDlms::decode(data);
    REQUIRE(ok(r));
    REQUIRE_FALSE(r.fields.values.empty());
    REQUIRE(r.fields.values[0].identifier == "get-request-normal");
}

TEST_CASE("XDlms::decode - Despacha corretamente para GET-RESPONSE", "[XDlms]")
{
    auto data = hexToBytes("C4 01 41 01 00");
    auto r = XDlms::decode(data);
    REQUIRE(ok(r));
    REQUIRE_FALSE(r.fields.values.empty());
    REQUIRE(r.fields.values[0].identifier == "get-response-normal");
}

TEST_CASE("XDlms::decode - Despacha corretamente para SET-REQUEST", "[XDlms]")
{
    auto data = hexToBytes("C1 01 41 00 01 01 00 01 08 00 FF 02 00 09 01 AA");
    auto r = XDlms::decode(data);
    REQUIRE(ok(r));
    REQUIRE(r.fields.values[0].identifier == "set-request-normal");
}

TEST_CASE("XDlms::decode - Despacha corretamente para SET-RESPONSE", "[XDlms]")
{
    auto data = hexToBytes("C5 01 41 00");
    auto r = XDlms::decode(data);
    REQUIRE(ok(r));
    REQUIRE(r.fields.values[0].identifier == "set-response-normal");
}

TEST_CASE("XDlms::decode - Despacha corretamente para ACTION-REQUEST", "[XDlms]")
{
    auto data = hexToBytes("C3 01 41 00 01 01 00 01 08 00 FF 01 00");
    auto r = XDlms::decode(data);
    REQUIRE(ok(r));
    REQUIRE(r.fields.values[0].identifier == "action-request-normal");
}

TEST_CASE("XDlms::decode - Despacha corretamente para ACTION-RESPONSE", "[XDlms]")
{
    auto data = hexToBytes("C7 01 41 00 00");
    auto r = XDlms::decode(data);
    REQUIRE(ok(r));
    REQUIRE(r.fields.values[0].identifier == "action-response-normal");
}
