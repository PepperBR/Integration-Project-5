#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <string>
#include <vector>

#include "core/CommandTypes/ACTIONS/RESPONSE/ActionResponseParser.h"
#include "hexToBytes.h"

TEST_CASE("ActionResponseParser - Validações Gerais e Erros de Inicialização", "[ActionResponse][General]")
{
    ActionResponseParser parser;

    SECTION("Frame excessivamente curto (< 3 bytes)")
    {
        std::vector<uint8_t> data = {0xC7, 0x01};
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Subtipo de ACTION-RESPONSE desconhecido")
    {
        auto data = hexToBytes("C7 09 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-NORMAL (Subtipo 0x01)", "[ActionResponse][Normal]")
{
    ActionResponseParser parser;

    SECTION("Caminho Feliz: Sucesso e sem parâmetros de retorno")
    {
        auto data = hexToBytes("C7 01 41 00 00");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
        REQUIRE(response.fields.size() >= 3);
    }

    SECTION("Caminho Feliz: Com parâmetros de retorno (Get-Data-Result tipo Data)")
    {
        auto data = hexToBytes("C7 01 41 00 01 00 AA BB CC");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 4);
    }

    SECTION("Caminho Feliz: Com parâmetros de retorno (Get-Data-Result tipo Data-Access-Result)")
    {
        auto data = hexToBytes("C7 01 41 00 01 01 03");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 4);
    }

    SECTION("Erro: Frame incompleto / truncado no Action-Result")
    {
        auto data = hexToBytes("C7 01 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Action-Result com código inválido/desconhecido")
    {
        auto data = hexToBytes("C7 01 41 55");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Indicador de Return-Parameters inválido")
    {
        auto data = hexToBytes("C7 01 41 00 05");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-WITH-PBLOCK (Subtipo 0x02)", "[ActionResponse][WithPblock]")
{
    ActionResponseParser parser;

    SECTION("Caminho Feliz: Bloco de dados válido")
    {
        auto data = hexToBytes("C7 02 41 01 00 00 00 02 11 22");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 2);
    }

    SECTION("Erro: Frame muito curto para conter a estrutura mínima de um pblock")
    {
        auto data = hexToBytes("C7 02 41 01 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: DataBlock-SA incompleto no offset interno")
    {
        auto data = hexToBytes("C7 02 41 01 00 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-WITH-LIST (Subtipo 0x03)", "[ActionResponse][WithList]")
{
    ActionResponseParser parser;

    SECTION("Caminho Feliz: Lista com múltiplos resultados")
    {
        auto data = hexToBytes("C7 03 41 02 00 00 02 01");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 6);
    }

    SECTION("Erro: Frame menor que o tamanho mínimo de lista")
    {
        auto data = hexToBytes("C7 03 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Lista truncada prematuramente")
    {
        auto data = hexToBytes("C7 03 41 02 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Action-Result inválido dentro de um elemento da lista")
    {
        auto data = hexToBytes("C7 03 41 01 FF");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("ActionResponseParser - ACTION-RESPONSE-NEXT-PBLOCK (Subtipo 0x04)", "[ActionResponse][NextPblock]")
{
    ActionResponseParser parser;

    SECTION("Caminho Feliz: Número de bloco decodificado com sucesso")
    {
        auto data = hexToBytes("C7 04 41 00 00 00 FF");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE_FALSE(response.fields.empty());

        auto lastField = response.fields.back();
        CHECK(lastField.name == "Block-Number");
        CHECK(lastField.value == "255");
    }

    SECTION("Erro: Falta de bytes para compor o Unsigned32 do Block-Number")
    {
        auto data = hexToBytes("C7 04 41 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}