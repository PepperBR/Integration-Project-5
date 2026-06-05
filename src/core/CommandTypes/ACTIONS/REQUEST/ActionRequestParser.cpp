#include "core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.h"
#include "core/enums.h"
#include <sstream>
#include <string>
#include <variant>

auto ActionRequestParser::buildHeader(const std::vector<uint8_t> &data, VerifyFrameResponse &response) -> bool
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

auto ActionRequestParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 3)
    {
        response.valid = false;
        response.errors.push_back({0, "Frame ACTION-REQUEST muito curto.", ""});
        return response;
    }

    switch (data[1])
    {
    case 0x01:
        return verifyNormal(data);
    case 0x02:
        return verifyNextPblock(data);
    case 0x03:
        return verifyWithList(data);
    case 0x04:
        return verifyWithFirstPblock(data);
    case 0x05:
        return verifyWithListAndFirstPblock(data);
    case 0x06:
        return verifyWithPblock(data);
    default:
        response.valid = false;
        response.errors.push_back({1, "Sub-tipo de ACTION-REQUEST desconhecido: " + std::to_string(data[1]), ""});
        return response;
    }
}

auto ActionRequestParser::parseCosemMethodDescriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
{
    constexpr size_t descriptorSize = 9;
    if (offset + descriptorSize > data.size())
        return ValidationError{1, "Dados insuficientes para Cosem-Method-Descriptor.", ""};

    uint16_t classId = (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];

    std::string obis = std::to_string(data[offset + 2]) + "." + std::to_string(data[offset + 3]) + "." + std::to_string(data[offset + 4]) + "." +
                       std::to_string(data[offset + 5]) + "." + std::to_string(data[offset + 6]) + "." + std::to_string(data[offset + 7]);

    int8_t methodId = static_cast<int8_t>(data[offset + 8]);

    return ParsedField{"Cosem-Method-Descriptor", static_cast<int>(offset), static_cast<int>(descriptorSize),
                       "ClassId: " + std::to_string(classId) + ", OBIS: " + obis + ", MethodId: " + std::to_string(methodId),
                       "Descritor de Método COSEM (ClassId, OBIS, MethodId)"};
}

auto ActionRequestParser::parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>
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

auto ActionRequestParser::verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 13)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-NORMAL incompleto.", ""});
        return response;
    }

    auto descResult = parseCosemMethodDescriptor(data, 3);
    if (std::holds_alternative<ValidationError>(descResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(descResult));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(descResult));

    size_t paramOffset = 12;
    uint8_t hasParams = data[paramOffset];
    if (hasParams == 0x00)
    {
        response.fields.push_back({"Method-Invocation-Parameters", static_cast<int>(paramOffset), 1, "0", "Sem parâmetros de invocação"});
        paramOffset++;

        if (paramOffset < data.size())
        {
            response.valid = false;
            response.errors.push_back({2, "Erro estrutural: Bytes extras detectados em um frame ACTION-NORMAL sem parametros.", ""});
            return response;
        }
    }
    else if (hasParams == 0x01)
    {
        response.fields.push_back({"Method-Invocation-Parameters", static_cast<int>(paramOffset), 1, "1", "Parâmetros de invocação presentes"});
        paramOffset++;

        if (paramOffset >= data.size())
        {
            response.valid = false;
            response.errors.push_back({1, "ACTION-REQUEST-NORMAL invalido: Dados de parametros ausentes após indicador ativo.", ""});
            return response;
        }

        size_t paramLen = data.size() - paramOffset;
        response.fields.push_back({"Data (method parameters)", static_cast<int>(paramOffset), static_cast<int>(paramLen),
                                   std::to_string(paramLen) + " bytes", "Dados passados ao método COSEM"});
    }
    else
    {
        response.valid = false;
        response.errors.push_back({3, "Indicador de Method-Invocation-Parameters inválido: " + std::to_string(hasParams), ""});
    }

    return response;
}

auto ActionRequestParser::verifyNextPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 7)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-NEXT-PBLOCK incompleto.", ""});
        return response;
    }

    uint32_t blockNumber = (static_cast<uint32_t>(data[3]) << 24) | (static_cast<uint32_t>(data[4]) << 16) | (static_cast<uint32_t>(data[5]) << 8) |
                           static_cast<uint32_t>(data[6]);

    response.fields.push_back({"Block-Number", 3, 4, std::to_string(blockNumber), "Número do próximo bloco (pblock) solicitado"});

    if (data.size() > 7)
    {
        response.valid = false;
        response.errors.push_back({2, "Erro estrutural: Bytes extras detectados ao final do frame ACTION-REQUEST-NEXT-PBLOCK.", ""});
    }

    return response;
}

auto ActionRequestParser::verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-WITH-LIST incompleto.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Method-Descriptor-List Count", 3, 1, std::to_string(count), "Número de métodos na lista"});

    size_t offset = 4;
    for (uint8_t i = 0; i < count; ++i)
    {
        auto descResult = parseCosemMethodDescriptor(data, offset);
        if (std::holds_alternative<ValidationError>(descResult))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(descResult));
            return response;
        }
        auto field = std::get<ParsedField>(descResult);
        field.name = "Cosem-Method-Descriptor[" + std::to_string(i) + "]";
        response.fields.push_back(field);
        offset += 9;
    }

    if (offset >= data.size())
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-WITH-LIST invalido: Lista de parametros (Method-Invocation-Parameters) ausente.", ""});
        return response;
    }

    size_t paramLen = data.size() - offset;
    response.fields.push_back({"Method-Invocation-Parameters", static_cast<int>(offset), static_cast<int>(paramLen),
                               std::to_string(paramLen) + " bytes", "Lista de parâmetros de invocação dos métodos"});

    return response;
}

auto ActionRequestParser::verifyWithFirstPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 18)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-WITH-FIRST-PBLOCK incompleto.", ""});
        return response;
    }

    auto descResult = parseCosemMethodDescriptor(data, 3);
    if (std::holds_alternative<ValidationError>(descResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(descResult));
        return response;
    }
    response.fields.push_back(std::get<ParsedField>(descResult));

    auto blockResult = parseDataBlockSA(data, 12);
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

auto ActionRequestParser::verifyWithListAndFirstPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 4)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-WITH-LIST-AND-FIRST-PBLOCK incompleto.", ""});
        return response;
    }

    uint8_t count = data[3];
    response.fields.push_back({"Method-Descriptor-List Count", 3, 1, std::to_string(count), "Número de métodos na lista"});

    size_t offset = 4;
    for (uint8_t i = 0; i < count; ++i)
    {
        auto descResult = parseCosemMethodDescriptor(data, offset);
        if (std::holds_alternative<ValidationError>(descResult))
        {
            response.valid = false;
            response.errors.push_back(std::get<ValidationError>(descResult));
            return response;
        }
        auto field = std::get<ParsedField>(descResult);
        field.name = "Cosem-Method-Descriptor[" + std::to_string(i) + "]";
        response.fields.push_back(field);
        offset += 9;
    }

    auto blockResult = parseDataBlockSA(data, offset);
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

auto ActionRequestParser::verifyWithPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    if (!buildHeader(data, response))
        return response;

    if (data.size() < 9)
    {
        response.valid = false;
        response.errors.push_back({1, "ACTION-REQUEST-WITH-PBLOCK incompleto.", ""});
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