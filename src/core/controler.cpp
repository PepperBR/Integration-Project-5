#include "core/controler.h"

static std::vector<uint8_t> hexToBytes(const std::string &hex)
{
    std::vector<uint8_t> bytes;
    std::string clean;

    // Remove espaços e hífens
    for (char c : hex)
    {
        if (c != ' ' && c != '-')
        {
            clean += c;
        }
    }

    // Quantidade inválida de caracteres
    if (clean.size() % 2 != 0)
    {
        throw std::runtime_error("Quantidade ímpar de caracteres hexadecimais.");
    }

    for (size_t i = 0; i < clean.size(); i += 2)
    {
        if (!std::isxdigit(clean[i]) || !std::isxdigit(clean[i + 1]))
        {
            throw std::runtime_error("Caracter hexadecimal inválido encontrado.");
        }

        std::string byteString = clean.substr(i, 2);

        bytes.push_back(static_cast<uint8_t>(std::stoul(byteString, nullptr, 16)));
    }

    return bytes;
}

grpc::Status Controler::HandleVerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response)
{
    if (!request || !response)
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Request ou Response nulos.");
    }

    try
    {
        // Converte HEX textual para bytes binários
        std::vector<uint8_t> binary_frame = hexToBytes(request->raw_frame());

        if (binary_frame.empty())
        {
            response->set_valid(false);

            auto *error = response->add_errors();
            error->set_code(400);
            error->set_message("O payload hexadecimal 'raw_frame' está vazio.");

            return grpc::Status::OK;
        }

        // Executa verificação
        Verifier verifier;
        auto internal_result = verifier.validateData(binary_frame);

        // Status geral
        response->set_valid(internal_result.valid);

        // Campos parseados
        for (const auto &internal_field : internal_result.fields)
        {
            auto *proto_field = response->add_fields();

            proto_field->set_name(internal_field.name);
            proto_field->set_offset(internal_field.offset);
            proto_field->set_length(internal_field.length);
            proto_field->set_value(internal_field.value);
            proto_field->set_description(internal_field.description);
        }

        // Erros
        for (const auto &internal_err : internal_result.errors)
        {
            auto *proto_error = response->add_errors();

            proto_error->set_code(static_cast<uint32_t>(internal_err.offset));

            std::string detailed_message = internal_err.message;

            if (!internal_err.found.empty())
            {
                detailed_message += " Encontrado: " + internal_err.found;
            }

            proto_error->set_message(detailed_message);
        }

        return grpc::Status::OK;
    }
    catch (const std::exception &e)
    {
        response->set_valid(false);

        auto *error = response->add_errors();

        error->set_code(500);
        error->set_message(std::string("Falha ao processar frame: ") + e.what());

        return grpc::Status::OK;
    }
}

void Controler::HandleFrameToProto(std::shared_ptr<os::VerifyFrameRequest> frame, os::VerifyFrameResponse *proto_frame)
{
    if (frame && proto_frame)
    {
        HandleVerifyFrame(nullptr, frame.get(), proto_frame);
    }
}
