#include <condition_variable>
#include <csignal>
#include <iostream>

#include "config/config.h"
#include "server/server.h"
#include "service/frame_service.h"

std::unique_ptr<Server> g_server = nullptr;
bool shutdown_requested = false;
std::mutex shutdown_mutex;
std::condition_variable shutdown_cv;

void signal_handler(int signal)
{
    std::cout << "Received signal " << signal << std::endl;
    shutdown_requested = true;
    shutdown_cv.notify_one();
}

int main()
{
    try
    {
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        Config config = Config::New();
        auto oService = std::make_shared<FrameService>();

        g_server = std::make_unique<Server>(config.host + ":" + config.port, oService, os::FrameService::service_full_name());

        g_server->Start();

        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;

        {
            std::unique_lock<std::mutex> lock(shutdown_mutex);
            shutdown_cv.wait(lock, [] { return shutdown_requested; });
        }

        std::cout << "\nSignal received, initiating graceful shutdown..." << std::endl;

        if (g_server)
        {
            g_server->Stop();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Exiting program." << std::endl;
    return 0;
}