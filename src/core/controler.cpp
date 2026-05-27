#include "core/controler.h"
Status Controler::HandleVerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response)
{
    // TODO: waiting for the implementation of the Verifier class
}

void Controler::HandleFrameToProto(std::shared_ptr<os::Frame> frame, os::Frame *proto_frame)
{
}