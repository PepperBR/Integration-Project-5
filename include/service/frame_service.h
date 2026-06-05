#pragma once

#include "grpcpp/grpcpp.h"
#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"

#include "frame.grpc.pb.h"
#include "frame.pb.h"

namespace os = frame::v1;

class FrameService final : public os::FrameService::Service
{
public:
    grpc::Status VerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response) override;

private:
    template <typename Request, typename Response>
    grpc::Status execute(const Request *request, Response *response, std::function<void()> f)
    {
        try
        {
            f();

            return grpc::Status::OK;
        }
        catch (const std::invalid_argument &e)
        {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
        }
        catch (const std::exception &e)
        {
            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        }
    }
};
