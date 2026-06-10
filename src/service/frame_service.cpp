#include "service/frame_service.h"
#include "core/XDlms.h"
#include "core/controller.h"

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

Status FrameService::VerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequestProto *request, os::VerifyFrameResponseProto *response)
{
    if (!request || !response)
    {
        return Status(StatusCode::INVALID_ARGUMENT, "Request ou Response nulos.");
    }

    return execute(request, response, [&]() {
        try
        {
            Controller::HandleVerifyFrame(request, response);
        }
        catch (const std::exception &e)
        {
            auto *error_proto = response->mutable_error();
            error_proto->set_message(std::string("Falha ao processar frame: ") + e.what());
        }
    });
}