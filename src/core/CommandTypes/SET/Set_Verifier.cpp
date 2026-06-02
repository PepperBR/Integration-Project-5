#include "core/CommandTypes/SET/SET_Verifier.h"

auto Set_Verifier::typeVerifier(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    uint8_t serviceTag = data[0];

    ParsedField subTypeField;
    subTypeField.offset = 1;
    subTypeField.length = 1;

    if (serviceTag == 0xC1)
    {
        SET_REQUEST_VERIFIER request_verifier;
        return request_verifier.verify(data);
    }
    else if (serviceTag == 0xC5)
    {
        SET_RESPONSE_VERIFIER response_verifier;
        return response_verifier.verify(data);
    }

    response.valid = false;
    ValidationError err;
    err.offset = 0;
    err.message = "Tag de comando principal inválida para o contexto de SET (Esperado 0xC1 ou 0xC5).";
    response.errors.push_back(err);
    return response;
}