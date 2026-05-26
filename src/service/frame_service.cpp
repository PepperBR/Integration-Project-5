#include "service/frame_service.h"

Status FrameService::VerifyFrame(ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response)
{
    return this->controler.HandleVerifyFrame(context, request, response);
}