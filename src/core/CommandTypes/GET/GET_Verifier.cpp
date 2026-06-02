#include "core/CommandTypes/GET/GET_Verifier.h"

auto Get_Verifier::typeVerifier(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    uint8_t serviceTag = data[0];

    ParsedField subTypeField;
    subTypeField.offset = 1;
    subTypeField.length = 1;

    if (serviceTag == 0xC0)
    {
        GET_REQUEST_VERIFIER request_verifier;
        return request_verifier.verify(data);
    }
    else if (serviceTag == 0xC4)
    {
        // GET_RESPONSE_Verifier response_verifier;
        // return response_verifier.verify(data);
    }

    response.valid = false;
    ValidationError err;
    err.offset = 0;
    err.message = "Tag de comando principal inválida para o contexto de GET.";
    response.errors.push_back(err);
    return response;
}