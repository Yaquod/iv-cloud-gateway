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

#include "mqtt_router.h"

#include <nlohmann/json.hpp>

#include "spdlog/spdlog.h"

namespace gateway::services {

void MqttRouter::on(const std::string& topic, Handler handler) {
  handlers_[topic] = std::move(handler);
}

void MqttRouter::dispatch(const std::string& topic,
                          const std::string& payload) {
  auto it = handlers_.find(topic);
  if (it == handlers_.end()) {
    spdlog::error("[MQTT Router] No handler registered for topic '{}'", topic);
    return;
  }
  try {
    it->second(payload);
  } catch (const nlohmann::json::exception& e) {
    spdlog::info("[MQTT Router] JSON error on topic '{}' : {} \n {}", topic,
                 e.what(), payload);
  } catch (const std::exception& e) {
    spdlog::error("[MQTT Router] exception on topic {} : {}", topic, e.what());
  }
}
}  // namespace gateway::services