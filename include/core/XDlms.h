#pragma once

#include <string>
#include <vector>

#include "core/CommonVerifierTypes.h"
#include "enums.h"

class XDlms
{
private:
    static auto to_hex_string(const unsigned char *bytes, int length) -> std::string;
    static auto verify_command(XDlmsApduTag commandType, const std::vector<uint8_t> &data, const int offset) -> FrameResponse;

public:
    static auto decode(const std::vector<uint8_t> &data) -> FrameResponse;
};
