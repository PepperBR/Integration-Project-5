#include "core/CommandTypes/ACTIONS/RESPONSE/ActionResponseParser.h"
#include "core/enums.h"
#include <sstream>
#include <string>
#include <variant>

auto ActionResponseParser::buildHeader(const std::vector<uint8_t> &data, VerifyFrameResponse &response) -> bool
{
    auto serviceType = static_cast<ServiceType>(data[1]);
    auto invokeInfo = ParseHeader::decodeInvokeField(data[2]);
    auto headerResult = ParseHeader::parse_header(serviceType, invokeInfo.priority, invokeInfo.serviceClass);

    if (std::holds_alternative<ValidationError>(headerResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(headerResult));
        return false;
    }
    response.fields.push_back(std::get<ParsedField>(headerResult));
    return true;
}

auto ActionResponseParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 3)
    {
        response.valid = false;
        response.errors.push_back({0, "Frame ACTION-RESPONSE muito curto.", ""});
        return response;
    }

    switch (data[1])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyWithPblock(data);
    case 0x03:
        return verifyWithList(data);
    case 0x04:
        return verifyNextPblock(data);
    default:
        response.valid = false;
        response.errors.push_back({1, "Sub-tipo de ACTION-RESPONSE desconhecido: " + std::to_string(data[1]), ""});
        return response;
    }
}

auto ActionResponseParser::parseActionResult(uint8_t value, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    struct Entry
    {
        uint8_t code;
        const char *label;
    };
    static constexpr Entry table[] = {
        {0, "Success"},
        {1, "Hardware Fault"},
        {2, "Temporary Failure"},
        {3, "Read/Write Denied"},
        {4, "Object Undefined"},
        {9, "Object Class Inconsistent"},
        {11, "Object Unavailable"},
        {12, "Type Unmatched"},
        {13, "Scope Of Access Violated"},
        {14, "Data Block Unavailable"},
        {15, "Long Action Aborted"},
        {16, "No Long Action In Progress"},
        {250, "Other Reason"},
    };

    for (const auto &e : table)
    {
        if (e.code == value)
            return ParsedField{"Action-Result", static_cast<int>(offset), 1, std::to_string(value), e.label};
    }
    return ValidationError{2, "Action-Result desconhecido: " + std::to_string(value), ""};
}

auto ActionResponseParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    if (offset + 6 > data.size())
        return ValidationError{1, "DataBlock-SA (pblock) incompleto ou sem definicao de tamanho.", ""};

    bool lastBlock = data[offset] != 0x00;

    uint32_t blockNumber = (static_cast<uint32_t>(data[offset + 1]) << 24) | (static_cast<uint32_t>(data[offset + 2]) << 16) |
                           (static_cast<uint32_t>(data[offset + 3]) << 8) | static_cast<uint32_t>(data[offset + 4]);

    size_t lengthOffset = offset + 5;
    uint8_t declaredBlockLen = data[lengthOffset];
    size_t actualBlockLen = data.size() - (lengthOffset + 1);

    if (actualBlockLen != declaredBlockLen)
    {
        return ValidationError{2,
                               "Erro estrutural no DataBlock-SA (pblock): Tamanho declarado (" + std::to_string(declaredBlockLen) +
                                   " bytes) difere dos bytes reais recebidos (" + std::to_string(actualBlockLen) + " bytes).",
                               ""};
    }

    std::ostringstream oss;
    oss << "LastBlock: " << (lastBlock ? "true" : "false") << ", BlockNumber: " << blockNumber << ", RawData: " << actualBlockLen << " bytes";

    size_t totalBlockSize = data.size() - offset;
    return ParsedField{"DataBlock-SA (pblock)", static_cast<int>(offset), static_cast<int>(totalBlockSize), oss.str(),
                       "Bloco de dados para ACTION (last-block, block-number, raw-data)"};
}

auto ActionResponseParser::verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-RESPONSE-NORMAL incompleto.", ""});
        return response;
    }

    auto resultField = parseActionResult(data[3], 3);
    if (std::holds_alternative<ValidationError>(resultField))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(resultField));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(resultField));

    size_t offset = 4;
    if (data.size() > offset)
    {
        uint8_t hasReturn = data[offset];
        if (hasReturn == 0x00)
        {
            response.fields.push_back({"Return-Parameters", static_cast<int>(offset), 1, "0", "Sem parâmetros de retorno"});
            offset++;
        }
        else if (hasReturn == 0x01)
        {
            response.fields.push_back({"Return-Parameters", static_cast<int>(offset), 1, "1", "Parâmetros de retorno presentes"});
            offset++;

            if (offset >= data.size())
            {
                response.valid = false;
                response.errors.push_back({1, "ACTION-RESPONSE-NORMAL invalido: Get-Data-Result ausente.", ""});
                return response;
            }

            uint8_t choiceTag = data[offset];
            if (choiceTag == 0x00)
            {
                size_t payloadLen = data.size() - (offset + 1);
                response.fields.push_back({"Get-Data-Result (Data)", static_cast<int>(offset), static_cast<int>(payloadLen + 1),
                                           std::to_string(payloadLen) + " bytes", "Dados de retorno da ação"});
                offset = data.size();
            }
            else if (choiceTag == 0x01)
            {
                if (offset + 1 >= data.size())
                {
                    response.valid = false;
                    response.errors.push_back({1, "ACTION-RESPONSE-NORMAL invalido: Data-Access-Result do choice ausente.", ""});
                    return response;
                }

                auto dar = parseActionResult(data[offset + 1], offset + 1);
                if (std::holds_alternative<ValidationError>(dar))
                {
                    response.valid = false;
                    response.errors.push_back(std::get<ValidationError>(dar));
                    return response;
                }

                auto f = std::get<ParsedField>(dar);
                f.name = "Get-Data-Result (Data-Access-Result)";
                response.fields.push_back(f);
                offset += 2;
            }
            else
            {
                response.valid = false;
                response.errors.push_back({3, "Tag de escolha de Get-Data-Result invalida: " + std::to_string(choiceTag), ""});
                return response;
            }
        }
        else
        {
            response.valid = false;
            response.errors.push_back({3, "Indicador de Return-Parameters inválido: " + std::to_string(hasReturn), ""});
            return response;
        }
    }

    if (offset < data.size())
    {
        response.valid = false;
        response.errors.push_back({2, "Erro estrutural: Bytes extras detectados ao final do frame ACTION-RESPONSE-NORMAL.", ""});
    }

    return response;
}

auto ActionResponseParser::verifyWithPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 9)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-RESPONSE-WITH-PBLOCK incompleto.", ""});
        return response;
    }

    auto blockResult = parseDataBlockSA(data, 3);
    if (std::holds_alternative<ValidationError>(blockResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(blockResult));
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(blockResult));
    }

    return response;
}

auto ActionResponseParser::verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-RESPONSE-WITH-LIST incompleto.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Response-List Count", 3, 1, std::to_string(count), "Número de respostas na lista"});

    size_t offset = 4;
    for (uint8_t i = 0; i < count; ++i)
    {
        if (offset >= data.size())
        {
            response.valid = false;
            response.errors.push_back({1, "Lista de respostas truncada no índice " + std::to_string(i), ""});
            return response;
        }

        auto resultField = parseActionResult(data[offset], offset);
        if (std::holds_alternative<ValidationError>(resultField))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(resultField));
            return response;
        }
        auto f = std::get<ParsedField>(resultField);
        f.name = "Action-Result[" + std::to_string(i) + "]";
        response.fields.push_back(f);
        offset++;

        if (offset >= data.size())
        {
            response.valid = false;
            response.errors.push_back({1, "Indicador Return-Parameters ausente na lista no índice " + std::to_string(i), ""});
            return response;
        }

        uint8_t hasReturn = data[offset++];
        ParsedField rf{"Return-Parameters[" + std::to_string(i) + "]", static_cast<int>(offset - 1), 1, std::to_string(hasReturn),
                       hasReturn ? "Parâmetros de retorno presentes" : "Sem parâmetros de retorno"};
        response.fields.push_back(rf);
    }

    return response;
}

auto ActionResponseParser::verifyNextPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 7)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-RESPONSE-NEXT-PBLOCK incompleto.", ""});
        return response;
    }

    uint32_t blockNumber = (static_cast<uint32_t>(data[3]) << 24) | (static_cast<uint32_t>(data[4]) << 16) | (static_cast<uint32_t>(data[5]) << 8) |
                           static_cast<uint32_t>(data[6]);

    response.fields.push_back({"Block-Number", 3, 4, std::to_string(blockNumber), "Número do próximo pblock solicitado pelo servidor"});

    if (data.size() > 7)
    {
        response.valid = false;
        response.errors.push_back({2, "Erro estrutural: Bytes extras detectados ao final do frame ACTION-RESPONSE-NEXT-PBLOCK.", ""});
    }

    return response;
}