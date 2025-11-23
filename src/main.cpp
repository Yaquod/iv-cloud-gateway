// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include <iostream>

#include "gateway.h"
#include <spdlog/spdlog.h>


bool shutdown_req = false;
std::mutex shutdown_mutex;
std::condition_variable shutdown_cv;

void signal_handler(int sig) {

    spdlog::info("shutdown signal is called");
    shutdown_req = true;
    shutdown_cv.notify_one();
}

int main()
{
    cloud_gateway::Gateway gateway;
    gateway.initialize();

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::thread shutdown_thread([&]() {
        std::unique_lock<std::mutex> lock(shutdown_mutex);
        shutdown_cv.wait(lock,[&]() {
            return shutdown_req;
        }

            );
        gateway.shutdown();
    });

    gateway.run();
    shutdown_thread.join();

    return 0;
}
