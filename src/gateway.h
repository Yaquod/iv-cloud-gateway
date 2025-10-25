// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#ifndef GATEWAY_H
#define GATEWAY_H

#include <memory>

#include "http_client/http_client.h"
#include "mqtt_client/mqtt_client.h"

namespace cloud_gateway {
class Gateway {
 public:
  Gateway();
  ~Gateway();

  void initialize();
  void run();
  void shutdown();

 private:
  std::unique_ptr<HttpClient> httpClient;
  std::unique_ptr<MqttClient> mqttClient;

  void register_vehicle();
};
}  // namespace cloud_gateway