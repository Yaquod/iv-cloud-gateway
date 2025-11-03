// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#ifndef IV_CLOUD_GATEWAY_MQTT_CLIENT_H
#define IV_CLOUD_GATEWAY_MQTT_CLIENT_H

#include <boost/mqtt5.hpp>
#include <string>
#include <thread>

namespace cloud_gateway {
class MqttClient {
 public:
  MqttClient(const std::string& broker, uint16_t port,
             const std::string& clientId);

  void mqtt_connect();
  void mqtt_disconnect();
  bool mqtt_publish(const std::string& topic, const std::string& payload);
  bool mqtt_subscribe(const std::string& topic);
  void start_runner();

 private:
  boost::asio::io_context ioc_;
  std::thread runner_;
  boost::mqtt5::mqtt_client<boost::asio::ip::tcp::socket, std::monostate,
                            boost::mqtt5::logger>
      client_;

  std::string broker_;
  uint16_t port_;
  std::string clientId_;
};
}  // namespace cloud_gateway
#endif  // IV_CLOUD_GATEWAY_MQTT_CLIENT_H
