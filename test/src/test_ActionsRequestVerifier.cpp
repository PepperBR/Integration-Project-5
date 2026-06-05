#include "core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.h"
#include "hexToBytes.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

TEST_CASE("ActionRequestParser - Validações Gerais e Erros de Header", "[ActionRequest][Header]")
{
    ActionRequestParser parser;

    SECTION("Frame excessivamente curto")
    {
        std::vector<uint8_t> data = {0xC6, 0x01};
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Subtipo de ACTION-REQUEST inválido/desconhecido")
    {
        auto data = hexToBytes("C6 07 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-NORMAL (Subtipo 0x01)", "[ActionRequest][Normal]")
{
    ActionRequestParser parser;

    SECTION("Caminho Feliz: Sem parâmetros de invocação (hasParams == 0x00)")
    {
        auto data = hexToBytes("C6 01 41 00 46 00 00 60 03 0A FF 01 00");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
        REQUIRE(response.fields.size() >= 3);
    }

    SECTION("Caminho Feliz: Com parâmetros de invocação (hasParams == 0x01)")
    {
        auto data = hexToBytes("C6 01 41 00 46 00 00 60 03 0A FF 01 01 AA BB CC");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 4);
    }

    SECTION("Erro: Frame incompleto para o Method Descriptor")
    {

        auto data = hexToBytes("C6 01 41 00 46 00 00 60");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Erro: Indicador de parâmetros inválido")
    {
        auto data = hexToBytes("C6 01 41 00 46 00 00 60 03 0A FF 01 05");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-NEXT-PBLOCK (Subtipo 0x02)", "[ActionRequest][NextPblock]")
{
    ActionRequestParser parser;

    SECTION("Caminho Feliz")
    {
        auto data = hexToBytes("C6 02 41 00 00 00 05");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE_FALSE(response.fields.empty());
        auto lastField = response.fields.back();
        CHECK(lastField.name == "Block-Number");
        CHECK(lastField.value == "5");
    }

    SECTION("Erro: Frame sem os 4 bytes do Block Number")
    {
        auto data = hexToBytes("C6 02 41 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-WITH-LIST (Subtipo 0x03)", "[ActionRequest][WithList]")
{
    ActionRequestParser parser;

    SECTION("Caminho Feliz: Lista com 2 métodos e sem parâmetros residuais")
    {
        auto data = hexToBytes("C6 03 41 02 "
                               "00 08 00 01 02 03 04 05 01 "
                               "00 0F 06 07 08 09 0A 0B 02");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 4);
    }

    SECTION("Caminho Feliz: Lista com parâmetros residuais no final")
    {
        auto data = hexToBytes("C6 03 41 01 00 08 00 01 02 03 04 05 01 11 22 33 44");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        auto lastField = response.fields.back();
        CHECK(lastField.name == "Method-Invocation-Parameters");
        CHECK(lastField.value == "4 bytes");
    }

    SECTION("Erro: Quantidade informada maior que os dados disponíveis")
    {
        auto data = hexToBytes("C6 03 41 02 00 08 00 01 02 03 04 05 01");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-WITH-FIRST-PBLOCK (Subtipo 0x04)", "[ActionRequest][WithFirstPblock]")
{
    ActionRequestParser parser;

    SECTION("Caminho Feliz")
    {
        auto data = hexToBytes("C6 04 41 "
                               "00 01 01 02 03 04 05 06 01 "
                               "01 00 00 00 0A EE FF");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 3);
    }

    SECTION("Erro: Dados insuficientes para o DataBlock-SA")
    {
        auto data = hexToBytes("C6 04 41 00 01 01 02 03 04 05 06 01 01 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-WITH-LIST-AND-FIRST-PBLOCK (Subtipo 0x05)", "[ActionRequest][WithListAndFirstPblock]")
{
    ActionRequestParser parser;

    SECTION("Caminho Feliz")
    {
        auto data = hexToBytes("C6 05 41 01 "
                               "00 0A 01 01 01 01 01 01 03 "
                               "00 00 00 00 01");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 4);
    }
}

TEST_CASE("ActionRequestParser - ACTION-REQUEST-WITH-PBLOCK (Subtipo 0x06)", "[ActionRequest][WithPblock]")
{
    ActionRequestParser parser;

    SECTION("Caminho Feliz")
    {
        auto data = hexToBytes("C6 06 41 01 00 00 00 02 11 22 33");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 2);
    }

    SECTION("Erro: Frame menor que o mínimo exigido de 8 bytes")
    {
        auto data = hexToBytes("C6 06 41 01 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}