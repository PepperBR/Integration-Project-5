#pragma once

#include <chrono>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <time.h>
#include <unordered_map>
#include <vector>

#include "google/protobuf/map.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"

#include "core/controler.h"
#include "frame.grpc.pb.h"
#include "frame.pb.h"

namespace os = frame::v1;

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

class FrameService final : public os::FrameService::Service
{
public:
    grpc::Status VerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response) override;

private:
    Controler controler;
};