#include "core/verifier.h"
#include "core/DLMSTypes.hpp"
#include <iomanip>
#include <sstream>

enum class DLMSTags : uint8_t
{
    OCTECT_STRING = 0x09,
    GET_RESPONSE = 0xC4
};

static std::string toHexString(const unsigned char *bytes, int length)
{
    std::ostringstream oss;

    for (int i = 0; i < length; ++i)
    {
        oss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)bytes[i] << " ";
    }

    std::string res = oss.str();

    if (!res.empty())
    {
        res.pop_back();
    }

    return res;
}

auto Verifier::validateData(std::vector<uint8_t> data) -> VerifyFrameResponse
{
    static constexpr uint8_t OCTET_STRING_TAG = 0x09;
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 4)
    {
        response.valid = false;
        ValidationError err;

        err.offset = 0;
        err.message = "Frame muito curto para ser uma APDU DLMS válida.";
        err.found = toHexString((unsigned char *)data.data(), static_cast<int>(data.size()));
        response.errors.push_back(err);

        return response;
    }

    try
    {
        // APDU Command Tag
        uint8_t first_byte = data[0];
        ParsedField cmdField;
        cmdField.name = "APDU Command Tag";
        cmdField.offset = 0;
        cmdField.length = 1;

        bool isResponse = false;
        bool isRequest = false;

        if (first_byte == 0xC1)
        {
            cmdField.value = "0xC1";
            cmdField.description = "GET-Request";
            isRequest = true;
        }
        else if (first_byte == 0xC4)
        {
            cmdField.value = "0xC4";
            cmdField.description = "GET-Response";
            isResponse = true;
        }
        else
        {
            response.valid = false;
            ValidationError err;
            err.offset = 0;
            err.message = "Tipo de APDU não suportado.";

            std::ostringstream ss;
            ss << "0x" << std::hex << std::uppercase << (int)first_byte;
            err.found = ss.str();
            response.errors.push_back(err);

            return response;
        }
        response.fields.push_back(cmdField);

        // Type Tag
        uint8_t subType = data[1];
        ParsedField subTypeField;
        subTypeField.name = isResponse ? "Response Type" : "Request Type";
        subTypeField.offset = 1;
        subTypeField.length = 1;

        std::ostringstream subTypeSs;
        subTypeSs << "0x" << std::hex << std::uppercase << (int)subType;
        subTypeField.value = subTypeSs.str();

        bool validSubType = false;
        bool isWithList = false;

        if (isResponse)
        {
            switch (subType)
            {
            case 0x01:
                subTypeField.description = "GET-Response-Normal";
                validSubType = true;
                break;
            case 0x02:
                subTypeField.description = "GET-Response-With-DataBlock";
                validSubType = true;
                break;
            case 0x03:
            case 0x04:
                subTypeField.description = "GET-Response-With-List";
                validSubType = true;
                isWithList = true;
                break;
            }
        }
        else if (isRequest)
        {
            switch (subType)
            {
            case 0x01:
                subTypeField.description = "GET-Request-Normal";
                validSubType = true;
                break;
            case 0x02:
                subTypeField.description = "GET-Request-Next";
                validSubType = true;
                break;
            case 0x03:
                subTypeField.description = "GET-Request-With-List";
                validSubType = true;
                isWithList = true;
                break;
            }
        }

        if (!validSubType)
        {
            response.valid = false;
            ValidationError err;
            err.offset = 1;
            err.message = isResponse ? "Response Type inválido." : "Request Type inválido.";
            err.found = subTypeField.value;
            response.errors.push_back(err);

            return response;
        }
        response.fields.push_back(subTypeField);

        // Invoke ID & Priority
        uint8_t invokeIdByte = data[2];
        ParsedField invokeField;
        invokeField.name = "Invoke ID & Priority";
        invokeField.offset = 2;
        invokeField.length = 1;

        std::ostringstream invokeSs;
        invokeSs << "0x" << std::hex << std::uppercase << (int)invokeIdByte;
        invokeField.value = invokeSs.str();

        int invokeId = invokeIdByte & 0x0F;
        std::string priority = (invokeIdByte & 0x80) ? "High" : "Normal";
        invokeField.description = "Invoke ID: " + std::to_string(invokeId) + " (" + priority + " Priority)";
        response.fields.push_back(invokeField);

        size_t payloadOffset = 4;

        if (isResponse && isWithList)
        {
            uint8_t listCount = data[3];
            ParsedField listCountField;
            listCountField.name = "Result List Count";
            listCountField.offset = 3;
            listCountField.length = 1;

            std::ostringstream countSs;
            countSs << "0x" << std::hex << std::uppercase << (int)listCount;
            listCountField.value = countSs.str();
            listCountField.description = "Lista contém " + std::to_string((int)listCount) + " item(ns)";
            response.fields.push_back(listCountField);

            if (data.size() > payloadOffset && data[payloadOffset] == 0x00)
            {
                ParsedField resultTypeField;
                resultTypeField.name = "GetDataResult Choice";
                resultTypeField.offset = static_cast<uint32_t>(payloadOffset);
                resultTypeField.length = 1;
                resultTypeField.value = "0x00";
                resultTypeField.description = "GetDataResult [0] -> Data (Success)";
                response.fields.push_back(resultTypeField);

                payloadOffset += 1;
            }
        }
        else
        {
            uint8_t resultCode = data[3];
            ParsedField resultField;
            resultField.name = "Result Code";
            resultField.offset = 3;
            resultField.length = 1;

            std::ostringstream resultSs;
            resultSs << "0x" << std::hex << std::uppercase << (int)resultCode;
            resultField.value = resultSs.str();

            switch (resultCode)
            {
            case 0x00:
                resultField.description = "Success";
                break;
            case 0x01:
                resultField.description = "Hardware Fault";
                break;
            case 0x02:
                resultField.description = "Temporary Failure";
                break;
            case 0x03:
                resultField.description = "Read Write Denied";
                break;
            default: {
                response.valid = false;
                ValidationError err;
                err.offset = 3;
                err.message = "Result Code desconhecido.";
                err.found = resultField.value;
                response.errors.push_back(err);

                return response;
            }
            }
            response.fields.push_back(resultField);
        }

        // Decodificação do COSEM Data Payload
        if (data.size() > payloadOffset)
        {
            uint8_t dataTag = data[payloadOffset];
            ParsedField payloadField;

            payloadField.name = "COSEM Data Payload";
            payloadField.offset = static_cast<uint32_t>(payloadOffset);
            payloadField.length = static_cast<uint32_t>(data.size() - payloadOffset);
            payloadField.value = toHexString((unsigned char *)data.data() + payloadOffset, static_cast<int>(data.size() - payloadOffset));
            bool payloadValid = true;

            switch (dataTag)
            {
            case (uint8_t)(DLMSTags::OCTECT_STRING): { // AQUI RAUL
                // pepper::DLMSTypes::parserOctetString(1);
                break;
            }
            case 0x11: { // UInt8
                if (data.size() < payloadOffset + 2)
                {
                    payloadValid = false;
                    ValidationError err;
                    err.offset = payloadOffset;
                    err.message = "UInt8 incompleto.";
                    response.errors.push_back(err);
                }
                payloadField.description = "UInt8";
                break;
            }
            case 0x01: { // Array DLMS
                if (data.size() < payloadOffset + 2)
                {
                    payloadValid = false;
                    ValidationError err;
                    err.offset = payloadOffset;
                    err.message = "Array DLMS incompleto.";
                    response.errors.push_back(err);
                    break;
                }
                uint8_t count = data[payloadOffset + 1];
                size_t bytesRestantes = data.size() - (payloadOffset + 2);
                size_t minimoRequerido = static_cast<size_t>(count) * 2;

                if (bytesRestantes < minimoRequerido)
                {
                    payloadValid = false;
                    ValidationError err;
                    err.offset = payloadOffset + 1;
                    err.message = "Array com elementos faltando.";
                    response.errors.push_back(err);
                }
                payloadField.description = "Array DLMS";
                break;
            }
            case 0x02: { // Structure DLMS
                if (data.size() < payloadOffset + 2)
                {
                    payloadValid = false;
                    ValidationError err;
                    err.offset = payloadOffset;
                    err.message = "Structure DLMS incompleta.";
                    response.errors.push_back(err);
                    break;
                }
                uint8_t count = data[payloadOffset + 1];
                size_t bytesRestantes = data.size() - (payloadOffset + 2);
                size_t minimoRequerido = static_cast<size_t>(count) * 2;

                if (bytesRestantes < minimoRequerido)
                {
                    payloadValid = false;
                    ValidationError err;
                    err.offset = payloadOffset + 1;
                    err.message = "Structure com elementos faltando.";
                    response.errors.push_back(err);
                }
                payloadField.description = "Structure DLMS";
                break;
            }
            default: {
                payloadValid = false;
                ValidationError err;
                err.offset = payloadOffset;
                std::ostringstream ss;
                ss << "Tag DLMS desconhecida: 0x" << std::hex << std::uppercase << (int)dataTag;
                err.message = ss.str();
                response.errors.push_back(err);
                payloadField.description = "Payload inválido";
                break;
            }
            }

            if (!payloadValid)
            {
                response.valid = false;
            }
            response.fields.push_back(payloadField);
        }
    }
    catch (const std::exception &e)
    {
        response.valid = false;
        ValidationError err;
        err.offset = 0;
        err.message = std::string("Exceção inesperada no validador: ") + e.what();
        response.errors.push_back(err);
    }

    return response;
}