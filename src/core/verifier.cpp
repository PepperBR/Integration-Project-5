#include "core/verifier.h"

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
    VerifyFrameResponse response;
    response.valid = true;

    if (data.size() < 4)
    {
        response.valid = false;
        ValidationError err;

        err.offset = 0;

        err.message = "Frame muito curto para ser "
                      "uma APDU DLMS válida.";

        err.found = toHexString((unsigned char *)data.data(), static_cast<int>(data.size()));
        response.errors.push_back(err);

        return response;
    }

    try
    {
        uint8_t first_byte = data[0];
        ParsedField cmdField;

        cmdField.name = "APDU Command Tag";
        cmdField.offset = 0;
        cmdField.length = 1;

        if (first_byte == 0xC1)
        {
            cmdField.value = "0xC1";
            cmdField.description = "GET-Request";
        }
        else if (first_byte == 0xC4)
        {
            cmdField.value = "0xC4";
            cmdField.description = "GET-Response";
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

        uint8_t responseType = data[1];

        ParsedField responseField;

        responseField.name = "Response Type";
        responseField.offset = 1;
        responseField.length = 1;

        std::ostringstream responseSs;

        responseSs << "0x" << std::hex << std::uppercase << (int)responseType;

        responseField.value = responseSs.str();

        switch (responseType)
        {
        case 0x01:
            responseField.description = "GET-Response-Normal";
            break;

        case 0x02:
            responseField.description = "GET-Response-With-DataBlock";
            break;

        case 0x03:
            responseField.description = "GET-Response-With-List";
            break;

        default: {
            response.valid = false;
            ValidationError err;
            err.offset = 1;

            err.message = "Response Type inválido.";

            err.found = responseField.value;
            response.errors.push_back(err);

            return response;
        }
        }
        response.fields.push_back(responseField);
        ParsedField invokeField;
        invokeField.name = "Invoke ID & Priority";

        invokeField.offset = 2;
        invokeField.length = 1;

        std::ostringstream invokeSs;

        invokeSs << "0x" << std::hex << std::uppercase << (int)data[2];

        invokeField.value = invokeSs.str();
        invokeField.description = "Invoke ID";
        response.fields.push_back(invokeField);
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
        if (data.size() > 4)
        {
            size_t payloadOffset = 4;
            uint8_t dataTag = data[payloadOffset];
            ParsedField payloadField;

            payloadField.name = "COSEM Data Payload";
            payloadField.offset = static_cast<uint32_t>(payloadOffset);
            payloadField.length = static_cast<uint32_t>(data.size() - payloadOffset);
            payloadField.value = toHexString((unsigned char *)data.data() + payloadOffset, static_cast<int>(data.size() - payloadOffset));
            bool payloadValid = true;

            switch (dataTag)
            {
            case 0x09: {
                if (data.size() < payloadOffset + 2)
                {
                    payloadValid = false;
                    ValidationError err;
                    err.offset = payloadOffset;

                    err.message = "Octet String incompleta.";

                    response.errors.push_back(err);

                    break;
                }
                uint8_t declaredSize = data[payloadOffset + 1];

                size_t realSize = data.size() - (payloadOffset + 2);

                if (realSize < declaredSize)
                {
                    payloadValid = false;
                    ValidationError err;

                    err.offset = payloadOffset + 1;
                    std::ostringstream ss;

                    ss << "Octet String inválida. "
                       << "Tamanho declarado = " << (int)declaredSize << ", tamanho recebido = " << realSize;

                    err.message = ss.str();
                    response.errors.push_back(err);
                }
                payloadField.description = "Octet String";

                break;
            }
            case 0x11: {
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
            case 0x01: {
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
                size_t expectedMinimum = payloadOffset + 2 + (count * 2);

                if (data.size() < expectedMinimum)
                {
                    payloadValid = false;
                    ValidationError err;

                    err.offset = payloadOffset;

                    err.message = "Array com elementos faltando.";
                    response.errors.push_back(err);
                }
                payloadField.description = "Array DLMS";

                break;
            }
            case 0x02: {
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
                size_t expectedMinimum = payloadOffset + 2 + (count * 2);

                if (data.size() < expectedMinimum)
                {
                    payloadValid = false;
                    ValidationError err;

                    err.offset = payloadOffset;

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
