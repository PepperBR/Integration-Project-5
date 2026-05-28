#pragma once

#include "core/verifier.h"
#include "frame.pb.h"
#include "google/protobuf/map.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"
#include <iomanip>
#include <sstream>
#include <vector>

namespace os = frame::v1;

using grpc::Status;
using grpc::StatusCode;

class Controler
{
public:
    grpc::Status HandleVerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response);

    void HandleFrameToProto(std::shared_ptr<os::VerifyFrameRequest> frame, os::VerifyFrameResponse *proto_frame);

private:
};