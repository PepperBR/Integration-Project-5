#include <catch2/catch_test_macros.hpp>

#include "core/CommandTypes/GET/RESPONSE/GetResponseParser.h"
#include "hexToBytes.h"

TEST_CASE("GetResponseParser - Validações Iniciais de Borda", "[GetResponse][General]")
{
    GetResponseParser parser;

    SECTION("Frame excessivamente curto (< 3 bytes)")
    {
        std::vector<uint8_t> data = {0xC4, 0x01};
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Subtipo de GET-RESPONSE inválido/desconhecido")
    {
        auto data = hexToBytes("C4 0F 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}

TEST_CASE("GetResponseParser - GET-RESPONSE-NORMAL (Subtipo 0x01)", "[GetResponse][Normal]")
{
    GetResponseParser parser;

    SECTION("Caminho Feliz: Choice [0] - Retorno com Dados puros válidos")
    {
        auto data = hexToBytes("C4 01 41 00 AA BB CC");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }

    SECTION("Caminho Feliz: Choice [1] - Retorno com Data-Access-Result mapeado")
    {
        auto data = hexToBytes("C4 01 41 01 03");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
        REQUIRE(response.fields.size() == 1);
    }

    SECTION("Erro: Frame menor que o tamanho mínimo de 4 bytes")
    {
        auto data = hexToBytes("C4 01 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Tag de Choice [1] declarada mas sem o byte de enum subsequente")
    {
        auto data = hexToBytes("C4 01 41 01");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Tag de Choice inválida/desconhecida")
    {
        auto data = hexToBytes("C4 01 41 05");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Valor do enum Data-Access-Result desconhecido")
    {
        auto data = hexToBytes("C4 01 41 01 55");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("GetResponseParser - GET-RESPONSE-WITH-DATABLOCK (Subtipo 0x02)", "[GetResponse][WithDatablock]")
{
    GetResponseParser parser;

    SECTION("Caminho Feliz: Bloco finalizado com payload de dados puros")
    {
        auto data = hexToBytes("C4 02 41 01 00 00 00 05 00 11 22");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
        REQUIRE(response.fields.size() == 1);
    }

    SECTION("Caminho Feliz: Bloco intermediário retornando uma falha de acesso")
    {
        auto data = hexToBytes("C4 02 41 00 00 00 00 01 01 0E");
        auto response = parser.verify(data);

        REQUIRE(response.valid);
        REQUIRE(response.errors.empty());
    }

    SECTION("Erro: Tamanho total menor que a assinatura mínima estrutural de 9 bytes")
    {
        auto data = hexToBytes("C4 02 41 01 00 00 00 05");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: DataBlock-G incompleto na janela do offset interno")
    {
        auto data = hexToBytes("C4 02 41 01 00 00");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Result-Tag indica DAR mas o byte de enum não existe")
    {
        auto data = hexToBytes("C4 02 41 01 00 00 00 05 01");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }

    SECTION("Erro: Result-Tag interna inválida")
    {
        auto data = hexToBytes("C4 02 41 01 00 00 00 05 99");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
    }
}

TEST_CASE("GetResponseParser - GET-RESPONSE-WITH-LIST (Subtipo 0x03)", "[GetResponse][WithList]")
{
    GetResponseParser parser;

    SECTION("Mapeamento do recurso não implementado no firmware")
    {
        auto data = hexToBytes("C4 03 41");
        auto response = parser.verify(data);

        REQUIRE_FALSE(response.valid);
        REQUIRE_FALSE(response.errors.empty());
    }
}