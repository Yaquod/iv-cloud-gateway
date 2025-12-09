// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include <iostream>

#include "gateway.h"
#include <spdlog/spdlog.h>



std::atomic<bool> shutdown_req{false};
cloud_gateway::Gateway* global_gateway = nullptr;


void signal_handler(int sig)
{

    spdlog::info("shutdown signal is called");
    shutdown_req = true;

    if (global_gateway) {
        global_gateway->shutdown();
    }

}

int main()
{
    cloud_gateway::Gateway gateway;
    global_gateway = &gateway;
    gateway.initialize();

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);


    gateway.run();


    return 0;
}
