/*
 * Copyright 2026 wafdy
 * Copyright 2026 Alaa Hassan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IV_CLOUD_GATEWAY_MQTT_CLIENT_H
#define IV_CLOUD_GATEWAY_MQTT_CLIENT_H

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/mqtt5.hpp>
#include <functional>
#include <string>
#include <thread>

namespace gateway::transport {

using MessageHandler =
    std::function<void(const std::string& topic, const std::string& payload)>;

class MqttClient {
 public:
  MqttClient(std::string broker, uint16_t port, std::string clientId);

  ~MqttClient();

  void start();

  void stop();

  void publish(
      const std::string& topic, const std::string& payload,
      std::function<void(bool success, const std::string msg)> publish_cb);

  void subscribe(const std::string& topic);

  void set_message_handler(MessageHandler handler);

 private:
  std::unique_ptr<boost::asio::io_context> ioc_;
  std::thread runner_;
  std::unique_ptr<boost::mqtt5::mqtt_client<
      boost::asio::ip::tcp::socket, std::monostate, boost::mqtt5::logger>>
      client_;
  std::string broker_;
  uint16_t port_;
  std::string clientId_;
  MessageHandler cb_;
  std::atomic<bool> started_{false};
  std::atomic<bool> stopped_{false};
  std::vector<std::string> pending_topics_;

  void arm_receive();
  void do_subscribe(const std::string& topic);
};

}  // namespace gateway::transport
#endif  // IV_CLOUD_GATEWAY_MQTT_CLIENT_H
