#include "core/CommandTypes/ACTIONS/ACTIONS_Verifier.h"

auto Actions_Verifier::typeVerifier(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;

    uint8_t serviceTag = data[0];

    if (serviceTag == 0xC6)
    {
        ACTIONS_REQUEST_VERIFIER request_verifier;
        return request_verifier.verify(data);
    }
    else if (serviceTag == 0xC7)
    {
        ACTIONS_RESPONSE_VERIFIER response_verifier;
        return response_verifier.verify(data);
    }

    response.valid = false;
    ValidationError err;
    err.offset = 0;
    err.message = "Tag de comando principal inválida para o contexto de ACTIONS (Deve ser 0xC6 ou 0xC7).";
    response.errors.push_back(err);
    return response;
}