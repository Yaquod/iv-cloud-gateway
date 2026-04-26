// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include <spdlog/spdlog.h>

#include <iostream>

#include "auth_manager/auth_manager.h"
#include "gateway.h"

// std::atomic<bool> shutdown_req{false};
// Gateway* global_gateway = nullptr;

// NOTE: The following code is commented out because it requires refactoring to
// work with the new MQTT client implementation. The Gateway, AuthManager, and
// trip flow logic depend on the old MQTT client interface and need to be
// updated.
/*
extern void StopTripFlow();

void signal_handler(int sig) {
  spdlog::info("shutdown signal is called");
  shutdown_req = true;

  if (global_gateway) {
    global_gateway->shutdown();
  }
}
*/

// NOTE: The following code is commented out because it requires refactoring to
// work with the new MQTT client implementation. The Gateway, AuthManager, and
// trip flow logic depend on the old MQTT client interface and need to be
// updated.
/*
int main() {
  cloud_gateway::Gateway gateway;
  global_gateway = &gateway;
  gateway.initialize();

  HttpClient* http_client = gateway.get_http_client();
  AuthManager auth_manager(http_client);
  auth_manager.admin_setup();
  auth_manager.create_vehicles();

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::thread gateway_thread([&gateway]() { gateway.run(); });

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  start_trip_flow();

  gateway_thread.join();

  StopTripFlow();

  spdlog::info("Clean exit");

  return 0;
}
*/

int main() {
  spdlog::info(
      "This main is disabled for now. See comments above. Refactor required "
      "for new MQTT client integration.");
  return 0;
}
