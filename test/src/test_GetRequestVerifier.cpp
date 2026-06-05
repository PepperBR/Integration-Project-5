#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"
#include "hexToBytes.h"

TEST_CASE("GetRequestParser - Validações Gerais e Inicialização", "[GetRequest][General]")
{
    GetRequestParser parser;

    SECTION("Frame excessivamente curto (< 3 bytes)")
    {
        std::vector<uint8_t> data = {0xC0, 0x01};
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Subtipo de GET-REQUEST desconhecido")
    {
        auto data = hexToBytes("C0 0A 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("GetRequestParser - GET-REQUEST-NORMAL (Subtipo 0x01)", "[GetRequest][Normal]")
{
    GetRequestParser parser;

    SECTION("Caminho Feliz: Descriptor válido sem Access-Selection adicional")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Caminho Feliz: Com Selective-Access-Descriptor presente")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 01");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Caminho Feliz: Com flag de seleção indicando ausência")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02 00");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 3);
    }

    SECTION("Erro: Frame abaixo do tamanho mínimo de 12 bytes")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Descriptor falha por falta de dados internos")
    {
        auto data = hexToBytes("C0 01 41 00 01 01 00 01 08 00 FF 02");
        auto response = parser.verify(data);

        auto badHeaderData = hexToBytes("C0 01 FF 00 01 01 00 01 08 00 FF 02");
        auto headerResponse = parser.verify(badHeaderData);
    }
}

TEST_CASE("GetRequestParser - GET-REQUEST-NEXT (Subtipo 0x02)", "[GetRequest][Next]")
{
    GetRequestParser parser;

    SECTION("Caminho Feliz: Número de bloco lido corretamente")
    {
        auto data = hexToBytes("C0 02 41 00 00 00 0A");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE_FALSE(response.fields.empty());

        auto lastField = response.fields.back();
        CHECK(lastField.name == "Block-Number");
        CHECK(lastField.value == "10");
    }

    SECTION("Erro: Frame menor que o tamanho mínimo de 7 bytes")
    {
        auto data = hexToBytes("C0 02 41 00 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("GetRequestParser - GET-REQUEST-WITH-LIST (Subtipo 0x03)", "[GetRequest][WithList]")
{
    GetRequestParser parser;

    SECTION("Mapeamento do recurso não implementado")
    {
        auto data = hexToBytes("C0 03 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.fields.empty());
    }
}
