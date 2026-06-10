#pragma once

#include "core/XDlms.h"
#include "frame.pb.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"

namespace os = frame::v1;

class Controller
{
public:
    static grpc::Status HandleVerifyFrame(const os::VerifyFrameRequestProto *request, os::VerifyFrameResponseProto *response);
};
