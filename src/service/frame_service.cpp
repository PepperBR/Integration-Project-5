#include "service/frame_service.h"
#include "core/XDlms.h"
#include "core/controller.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

Status FrameService::VerifyFrame(ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response)
{
    // auto XDlms::decode(Controller::prepare_data(request));
    return execute(request, response, [&]() { Controller::HandleVerifyFrame(context, request, response); });
}