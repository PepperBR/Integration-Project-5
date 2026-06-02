#pragma once

#include <stdexcept>
#include <string>
#include <vector>

static std::vector<uint8_t> hexToBytes(const std::string &hex)
{
    std::vector<uint8_t> bytes;

    std::string clean;

    for (char c : hex)
    {
        if (c != ' ' && c != '-')
        {
            clean += c;
        }
    }

    if (clean.size() % 2 != 0)
    {
        throw std::runtime_error("Quantidade ímpar de caracteres hex.");
    }

    for (size_t i = 0; i < clean.size(); i += 2)
    {
        if (!std::isxdigit(static_cast<unsigned char>(clean[i])) ||

            !std::isxdigit(static_cast<unsigned char>(clean[i + 1])))
        {
            throw std::runtime_error("Hexadecimal inválido.");
        }

        bytes.push_back(static_cast<uint8_t>(std::stoul(clean.substr(i, 2), nullptr, 16)));
    }

    return bytes;
}
