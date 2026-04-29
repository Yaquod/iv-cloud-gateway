/*
 * Copyright 2026 wafdy
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

#ifndef VEHICLECLOUDGATEWAY_MQTT_ROUTER_H
#define VEHICLECLOUDGATEWAY_MQTT_ROUTER_H
#include <functional>
#include <string>

namespace gateway::services {

/**
 * @brief Routes inbound MQTT messages to per-topic handlers.
 */
class MqttRouter {
 public:
  using Handler = std::function<void(const std::string&)>;

  void on(const std::string& topic, Handler handler);

  /**
   * @brief Dispatches an incoming MQTT message, Called by MqttTransport's
   * message handler for every inbound message.
   * @param topic The topic of the incoming MQTT message
   * @param payload The payload sent by topic.
   */
  void dispatch(const std::string& topic, const std::string& payload);

 private:
  std::unordered_map<std::string, Handler> handlers_;
};

}  // namespace gateway::services
#endif  // VEHICLECLOUDGATEWAY_MQTT_ROUTER_H
