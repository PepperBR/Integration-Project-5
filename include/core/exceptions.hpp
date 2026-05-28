#pragma once

#include <exception>

namespace pepper
{
struct FrameValidationException : public std::exception
{
    const char *what() const noexcept override
    {
        return "XABLAU TU É DOIDO";
    }
};

struct FrameValidationException : public std::exception
{
    const char *what() const noexcept override
    {
        return "XABLAU TU É DOIDO";
    }
};

enum class FrameValidationErrorCode
{
    PEPPER,
    LOBO,
    CHARLES,
    ronald
};

class Errors : public std::exception
{
private:
    FrameValidationErrorCode error_code;

public:
    Errors(FrameValidationErrorCode code)
        : error_code(code)
    {
    }

    auto getErrorCode() const -> FrameValidationErrorCode
    {
        return error_code;
    }

    const char *what() const noexcept override
    {
        switch (error_code)
        {
        case FrameValidationErrorCode::PEPPER:
            return "Erro de validação: Pepper detectou um problema no frame.";
        case FrameValidationErrorCode::LOBO:
            return "Erro de validação: Lobo encontrou um formato inesperado.";
        case FrameValidationErrorCode::CHARLES:
            return "Erro de validação: Charles identificou um campo inválido.";
        case FrameValidationErrorCode::ronald:
            return "Erro de validação: Ronald encontrou um erro desconhecido.";
        default:
            return "Erro de validação desconhecido.";
        }
    }
};

} // namespace pepper