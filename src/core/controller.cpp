#include "core/controller.h"
#include "core/CommonVerifierTypes.h"

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
        throw std::runtime_error("O número de caracteres hexadecimais deve ser par.");
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

void MapFieldToProto(frame::v1::ParsedFieldProto *proto_field, const ParsedField &internal_field)
{
    proto_field->set_identifier(internal_field.identifier);
    proto_field->set_name(internal_field.name);
    proto_field->set_value_bytes(internal_field.value_bytes);

    for (const auto &child_field : internal_field.values)
    {
        auto *proto_child = proto_field->add_values();
        MapFieldToProto(proto_child, child_field);
    }
}

void GetFieldsInformation(frame::v1::VerifyFrameResponseProto *message, const FrameResponse &internal_result)
{
    MapFieldToProto(message->mutable_fields(), internal_result.fields);

    if (internal_result.error.has_value())
    {
        message->mutable_error()->set_message(internal_result.error->message);
    }
}

grpc::Status Controller::HandleVerifyFrame(const frame::v1::VerifyFrameRequestProto *request, frame::v1::VerifyFrameResponseProto *response)
{
    std::vector<uint8_t> binary_frame = hexToBytes(request->raw_frame());

    if (binary_frame.empty())
    {
        response->mutable_error()->set_message("O payload hexadecimal 'raw_frame' está vazio.");
        return grpc::Status::OK;
    }

    auto internal_result = XDlms::decode(binary_frame);

    GetFieldsInformation(response, internal_result);

    return grpc::Status::OK;
}