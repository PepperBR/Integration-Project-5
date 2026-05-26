#include "server/server.h"

Server::Server(std::string server_address, std::shared_ptr<grpc::Service> service, std::string service_name)
    : server_address_(server_address)
    , service_(service)
    , service_name_(service_name)
{
}

void Server::Start()
{
    grpc::ServerBuilder builder;

    builder.AddListeningPort(this->server_address_, grpc::InsecureServerCredentials());

    builder.RegisterService(this->service_.get());

    this->server_ = builder.BuildAndStart();

    std::cout << this->service_name_ << " listening on " << this->server_address_ << std::endl;
}

void Server::Wait()
{
    if (this->server_)
    {
        this->server_->Wait();
    }
}

void Server::Stop()
{
    std::cout << this->service_name_ << " shutting down, please wait..." << this->server_address_ << std::endl;
    this->server_->Shutdown();
    std::cout << this->service_name_ << " shutdown!" << std::endl;
}