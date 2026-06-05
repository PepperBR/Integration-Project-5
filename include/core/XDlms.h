#pragma once

#include <string>
#include <vector>

#include "core/CommonVerifierTypes.h"
#include "enums.h"

class XDlms
{
private:
    static auto to_hex_string(const unsigned char *bytes, int length) -> std::string;
    static auto verify_command(XDlmsDataType commandType, const std::vector<uint8_t> &data) -> VerifyFrameResponse;
    static auto identifier_x_dlms_data_type(uint8_t data) -> XDlmsDataType;

public:
    static auto decode(const std::vector<uint8_t> &data) -> VerifyFrameResponse;
};
