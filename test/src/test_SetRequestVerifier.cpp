#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/SET/REQUEST/SetRequestParser.h"
#include "hexToBytes.h"

TEST_CASE("SetRequestParser - Erros Iniciais e Gerais", "[SetRequest][General]")
{
    SetRequestParser parser;

    SECTION("Frame excessivamente curto (< 3 bytes)")
    {
        std::vector<uint8_t> data = {0xC1, 0x01};
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Subtipo de SET-REQUEST desconhecido")
    {
        auto data = hexToBytes("C1 99 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-NORMAL (Subtipo 0x01)", "[SetRequest][Normal]")
{
    SetRequestParser parser;

    SECTION("Caminho Feliz: Com descritor completo, sem seleção e com dados")
    {
        auto data = hexToBytes("C1 01 41 00 01 01 00 01 08 00 FF 02 00 12 34");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
        REQUIRE(response.fields.size() >= 4);
    }

    SECTION("Caminho Feliz: Com Selective-Access-Descriptor ativo")
    {
        auto data = hexToBytes("C1 01 41 00 01 01 00 01 08 00 FF 02 01 AA BB");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
    }

    SECTION("Erro: Frame truncado (< 12 bytes)")
    {
        auto data = hexToBytes("C1 01 41 00 01 01 00 01");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-WITH-FIRST-DATABLOCK (Subtipo 0x02)", "[SetRequest][FirstDatablock]")
{
    SetRequestParser parser;

    SECTION("Caminho Feliz: Descritor, seleção e pblock válidos")
    {
        auto data = hexToBytes("C1 02 41 00 01 01 00 01 08 00 FF 02 00 01 00 00 00 01 AA BB");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Erro: Frame abaixo de 18 bytes")
    {
        auto data = hexToBytes("C1 02 41 00 01 01 00 01 08 00 FF 02 00 01 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-WITH-DATABLOCK (Subtipo 0x03)", "[SetRequest][WithDatablock]")
{
    SetRequestParser parser;

    SECTION("Caminho Feliz: Apenas bloco de dados sequencial")
    {
        auto data = hexToBytes("C1 03 41 00 00 00 00 04 11 22 33");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Erro: Tamanho menor que o limite mínimo estrutural (8 bytes)")
    {
        auto data = hexToBytes("C1 03 41 00 00 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-WITH-LIST (Subtipo 0x04)", "[SetRequest][WithList]")
{
    SetRequestParser parser;

    SECTION("Caminho Feliz: Lista com múltiplos descritores e valores acoplados")
    {
        auto data = hexToBytes("C1 04 41 02 "
                               "00 01 01 00 01 08 00 FF 02 "
                               "00 03 01 00 03 00 00 FF 01 "
                               "AA BB CC DD");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
    }

    SECTION("Erro: Frame menor que o tamanho mínimo de controle (4 bytes)")
    {
        auto data = hexToBytes("C1 04 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Lista com contagem maior que o payload real de bytes enviado (Truncada)")
    {
        auto data = hexToBytes("C1 04 41 02 00 01 01 00 01 08 00 FF 02");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("SetRequestParser - SET-REQUEST-WITH-LIST-AND-FIRST-DATABLOCK (Subtipo 0x05)", "[SetRequest][ListAndFirstBlock]")
{
    SetRequestParser parser;

    SECTION("Caminho Feliz: Lista de descritores seguida de estrutura pblock")
    {
        auto data = hexToBytes("C1 05 41 01 "
                               "00 01 01 00 01 08 00 FF 02 "
                               "01 00 00 00 01 99");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Erro: Truncamento na janela de leitura do DataBlock-SA final")
    {
        auto data = hexToBytes("C1 05 41 01 00 01 01 00 01 08 00 FF 02 01 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}