#pragma once

struct ParsedField
{
    std::string name;
    int offset;
    int length;
    std::string value;
    std::string description;
};

struct ValidationError
{
    int offset;
    std::string message;
    std::string found;
};

struct VerifyFrameResponse
{
    bool valid;
    std::list<ParsedField> fields;
    std::list<ValidationError> errors;
};
