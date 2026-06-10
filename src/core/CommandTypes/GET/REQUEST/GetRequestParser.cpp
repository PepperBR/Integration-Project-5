#include "core/CommandTypes/GET/REQUEST/GetRequestParser.h"
#include "core/enums.h"
#include "core/utils/CosemDescriptorParser.h"
#include "core/utils/DlmsFrameUtils.h"

#include <string>
#include <variant>

static constexpr size_t PAYLOAD_OFFSET = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
static constexpr size_t DESCRIPTOR_SIZE = CosemDescriptorParser::DESCRIPTOR_SIZE;
static constexpr size_t SELECTION_OFFSET = PAYLOAD_OFFSET + DESCRIPTOR_SIZE;
static constexpr size_t BLOCK_NUMBER_SIZE = 4;

auto GetRequestParser::buildHeader(const std::vector<uint8_t> &data, FrameResponse &response) -> bool
{
    return DlmsFrameUtils::buildHeader(data, response);
}

auto GetRequestParser::verify(const std::vector<uint8_t> &data, int offset) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "get-request";
    response.fields.name = "Get-Request";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    constexpr size_t MIN_FRAME_SIZE = DlmsFrameUtils::APDU_PAYLOAD_OFFSET;
    if (data.size() < MIN_FRAME_SIZE)
    {
        response.error.emplace(Error{"Frame GET-REQUEST muito curto."});
        return response;
    }

    switch (data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])
    {
    case 0x01:
        return verifyNormal(data, offset);
    case 0x02:
        return verifyNext(data);
    case 0x03:
        response.error.emplace(Error{"GET-REQUEST-WITH-LIST não implementado: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    default:
        response.error.emplace(Error{"Sub-tipo de GET-REQUEST desconhecido: " + std::to_string(data[DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET])});
        return response;
    }
}

auto GetRequestParser::verifyNormal(const std::vector<uint8_t> &data, int offset) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "get-request-normal";
    response.fields.name = "Get-Request-Normal";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = SELECTION_OFFSET + 1;

    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"GET-REQUEST-NORMAL incompleto."});
        return response;
    }

    auto descriptorResult = Cosem_Attribute_Descriptor(data, PAYLOAD_OFFSET);
    if (std::holds_alternative<ParsedField>(descriptorResult))
    {
        response.fields.values.push_back(std::get<ParsedField>(descriptorResult));
    }
    else
    {
        response.error.emplace(std::get<Error>(descriptorResult));
        return response;
    }

    uint8_t hasSelection = data[SELECTION_OFFSET];
    ParsedField selField;
    selField.identifier = "access-selection";
    selField.name = "Access-Selection";
    selField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, SELECTION_OFFSET, 1);

    if (hasSelection == 0x01)
    {
        constexpr size_t selectionDataOffset = SELECTION_OFFSET + 1;
        if (data.size() <= selectionDataOffset)
        {
            response.error.emplace(Error{"Erro estrutural DLMS: Flag de selecao ativado (0x01), mas "
                                         "dados de selecao ausentes."});
            return response;
        }
        response.fields.values.push_back(selField);
    }
    else if (hasSelection == 0x00)
    {
        constexpr size_t expectedSize = SELECTION_OFFSET + 1;
        if (data.size() > expectedSize)
        {
            response.error.emplace(Error{"Erro estrutural DLMS: Bytes extras detectados em um frame sem "
                                         "selecao de acesso."});
            return response;
        }
        response.fields.values.push_back(selField);
    }
    else
    {
        response.error.emplace(Error{"Valor invalido para flag Access-Selection: " + std::to_string(hasSelection)});
        return response;
    }

    return response;
}

auto GetRequestParser::verifyNext(const std::vector<uint8_t> &data) -> FrameResponse
{
    FrameResponse response;
    response.fields.identifier = "get-request-next";
    response.fields.name = "Get-Request-Next";
    response.fields.value_bytes = DlmsFrameUtils::bytes_to_hex(data, DlmsFrameUtils::APDU_SERVICE_TYPE_OFFSET, 1);

    if (!buildHeader(data, response))
        return response;

    constexpr size_t minimumSize = PAYLOAD_OFFSET + BLOCK_NUMBER_SIZE; // 7

    if (data.size() < minimumSize)
    {
        response.error.emplace(Error{"GET-REQUEST-NEXT incompleto."});
        return response;
    }

    uint32_t blockNumber = DlmsFrameUtils::read_uint32_be(data, PAYLOAD_OFFSET);

    ParsedField bnField;
    bnField.identifier = "block-number";
    bnField.name = "Block-Number";
    bnField.value_bytes = DlmsFrameUtils::bytes_to_hex(data, PAYLOAD_OFFSET, BLOCK_NUMBER_SIZE);
    response.fields.values.push_back(bnField);

    return response;
}

auto GetRequestParser::Cosem_Attribute_Descriptor(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, Error>
{
    return CosemDescriptorParser::parseCosemAttributeDescriptor(data, offset);
}
