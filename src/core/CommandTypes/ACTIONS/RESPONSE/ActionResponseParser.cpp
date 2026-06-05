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

auto ActionResponseParser::parseActionResult(uint8_t value) -> std::variant<ParsedField, ValidationError>
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
            return ParsedField{"Action-Result", 3, 1, std::to_string(value), e.label};
    }
    return ValidationError{2, "Action-Result desconhecido: " + std::to_string(value), ""};
}

auto ActionResponseParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    if (offset + 5 > data.size())
        return ValidationError{1, "DataBlock-SA (pblock) incompleto.", ""};

    bool lastBlock = data[offset] != 0x00;

    uint32_t blockNumber = (static_cast<uint32_t>(data[offset + 1]) << 24) | (static_cast<uint32_t>(data[offset + 2]) << 16) |
                           (static_cast<uint32_t>(data[offset + 3]) << 8) | static_cast<uint32_t>(data[offset + 4]);

    size_t rawLen = data.size() - offset - 5;

    std::ostringstream oss;
    oss << "LastBlock: " << (lastBlock ? "true" : "false") << ", BlockNumber: " << blockNumber << ", RawData: " << rawLen << " bytes";

    return ParsedField{"DataBlock-SA (pblock)", static_cast<int>(offset), static_cast<int>(data.size() - offset), oss.str(),
                       "Bloco de dados para ACTION (last-block, block-number, raw-data)"};
}

auto ActionResponseParser::verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = 4;
    if (data.size() < minimumSize)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-RESPONSE-NORMAL incompleto.", ""});
        return response;
    }

    auto resultField = parseActionResult(data[3]);
    if (std::holds_alternative<ValidationError>(resultField))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(resultField));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(resultField));

    if (data.size() > 4)
    {
        uint8_t hasReturn = data[4];
        if (hasReturn == 0x00)
        {
            response.fields.push_back({"Return-Parameters", 4, 1, "0", "Sem parâmetros de retorno"});
        }
        else if (hasReturn == 0x01)
        {
            response.fields.push_back({"Return-Parameters", 4, 1, "1", "Parâmetros de retorno presentes"});

            if (data.size() > 5)
            {
                uint8_t choiceTag = data[5];
                if (choiceTag == 0x00)
                {
                    size_t payloadLen = data.size() - 6;
                    response.fields.push_back({"Get-Data-Result (Data)", 5, static_cast<int>(payloadLen + 1), std::to_string(payloadLen) + " bytes",
                                               "Dados de retorno da ação"});
                }
                else if (choiceTag == 0x01 && data.size() > 6)
                {
                    auto dar = parseActionResult(data[6]);
                    if (std::holds_alternative<ParsedField>(dar))
                    {
                        auto f = std::get<ParsedField>(dar);
                        f.name = "Get-Data-Result (Data-Access-Result)";
                        f.offset = 6;
                        response.fields.push_back(f);
                    }
                }
            }
        }
        else
        {
            response.valid = false;
            response.errors.push_back({3, "Indicador de Return-Parameters inválido.", ""});
        }
    }

    return response;
}

auto ActionResponseParser::verifyWithPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 8)
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
        auto resultField = parseActionResult(data[offset]);
        if (std::holds_alternative<ValidationError>(resultField))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(resultField));
            return response;
        }
        auto f = std::get<ParsedField>(resultField);
        f.name = "Action-Result[" + std::to_string(i) + "]";
        f.offset = static_cast<int>(offset);
        response.fields.push_back(f);
        offset++;

        if (offset < data.size())
        {
            uint8_t hasReturn = data[offset++];
            ParsedField rf{"Return-Parameters[" + std::to_string(i) + "]", static_cast<int>(offset - 1), 1, std::to_string(hasReturn),
                           hasReturn ? "Parâmetros de retorno presentes" : "Sem parâmetros de retorno"};
            response.fields.push_back(rf);
        }
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

    return response;
}
