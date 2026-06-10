#pragma once

#include <optional>
#include <string>
#include <vector>

struct ParsedField
{
    std::string identifier;
    std::string name;
    std::string value_bytes;
    std::vector<ParsedField> values;
};

struct Error
{
    std::string message;
};

struct FrameResponse
{
    ParsedField fields;
    std::optional<Error> error;
};
