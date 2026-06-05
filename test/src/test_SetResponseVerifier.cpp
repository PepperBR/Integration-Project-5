#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/SET/RESPONSE/SetResponseParser.h"
#include "hexToBytes.h"

TEST_CASE("SetResponseParser - Validações Gerais", "[SetResponse][General]")
{
    SetResponseParser parser;

    SECTION("Frame excessivamente curto (< 3 bytes)")
    {
        std::vector<uint8_t> data = {0xC5, 0x01};
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Subtipo de SET-RESPONSE desconhecido")
    {
        auto data = hexToBytes("C5 0A 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-NORMAL (Subtipo 0x01)", "[SetResponse][Normal]")
{
    SetResponseParser parser;

    SECTION("Caminho Feliz: Mapeamento de sucesso")
    {
        auto data = hexToBytes("C5 01 41 00");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 2);
    }

    SECTION("Erro: Frame incompleto (< 4 bytes)")
    {
        auto data = hexToBytes("C5 01 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Código Data-Access-Result desconhecido")
    {
        auto data = hexToBytes("C5 01 41 AA");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-DATABLOCK (Subtipo 0x02)", "[SetResponse][Datablock]")
{
    SetResponseParser parser;

    SECTION("Caminho Feliz: Leitura correta do Block-Number")
    {
        auto data = hexToBytes("C5 02 41 00 00 00 0C");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
    }

    SECTION("Erro: Frame incompleto (< 7 bytes)")
    {
        auto data = hexToBytes("C5 02 41 00 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-LAST-DATABLOCK (Subtipo 0x03)", "[SetResponse][LastDatablock]")
{
    SetResponseParser parser;

    SECTION("Caminho Feliz: Resultado do acesso seguido do número do bloco")
    {
        auto data = hexToBytes("C5 03 41 03 00 00 00 02");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.fields.size() >= 3);
    }

    SECTION("Erro: Frame incompleto (< 8 bytes)")
    {
        auto data = hexToBytes("C5 03 41 00 00 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-LAST-DATABLOCK-WITH-LIST (Subtipo 0x04)", "[SetResponse][LastDatablockWithList]")
{
    SetResponseParser parser;

    SECTION("Caminho Feliz: Múltiplos resultados na lista e Block-Number dinâmico no final")
    {
        auto data = hexToBytes("C5 04 41 02 01 04 00 00 00 07");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
    }

    SECTION("Erro: Frame menor que o tamanho inicial estrutural de 9 bytes")
    {
        auto data = hexToBytes("C5 04 41 01 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Lista de resultados declarada maior que os bytes reais existentes")
    {
        auto data = hexToBytes("C5 04 41 03 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Block-Number ausente após o processamento da lista de resultados")
    {
        auto data = hexToBytes("C5 04 41 01 00 00 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetResponseParser - SET-RESPONSE-WITH-LIST (Subtipo 0x05)", "[SetResponse][WithList]")
{
    SetResponseParser parser;

    SECTION("Caminho Feliz: Processamento limpo de lista de erros")
    {
        auto data = hexToBytes("C5 05 41 03 0B 0D 00");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
    }

    SECTION("Erro: Frame abaixo do tamanho mínimo de checagem (5 bytes)")
    {
        auto data = hexToBytes("C5 05 41 01");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}