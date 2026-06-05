#pragma once

#include "core/CommandTypes/ParseHeader.h"
#include "core/CommonVerifierTypes.h"
#include <cstdint>
#include <variant>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// ACTION-Response parser
//
// Action-Response ::= CHOICE {
//   [1] action-response-normal
//   [2] action-response-with-pblock
//   [3] action-response-with-list
//   [4] action-response-next-pblock
// }
// ─────────────────────────────────────────────────────────────────────────────
class ActionResponseParser
{
public:
    static auto verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse;

private:
    static auto verifyNormal(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyWithList(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto verifyNextPblock(const std::vector<uint8_t> &data) -> VerifyFrameResponse;

    static auto parseActionResult(uint8_t value) -> std::variant<ParsedField, ValidationError>;

    static auto parseDataBlockSA(const std::vector<uint8_t> &data, size_t offset) -> std::variant<ParsedField, ValidationError>;

    static auto buildHeader(const std::vector<uint8_t> &data, VerifyFrameResponse &response) -> bool;
};
