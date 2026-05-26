#pragma once

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>

class Server
{
public:
    explicit Server(std::string server_address, std::shared_ptr<grpc::Service> service = nullptr, std::string service_name = "Server");

    void Start();
    void Stop();
    void Wait();

private:
    std::unique_ptr<grpc::Server> server_;
    std::shared_ptr<grpc::Service> service_;
    std::string server_address_;
    std::string service_name_;
};