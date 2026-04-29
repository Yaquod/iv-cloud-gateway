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

#ifndef VEHICLECLOUDGATEWAY_TRIP_ORCHESTRATOR_H
#define VEHICLECLOUDGATEWAY_TRIP_ORCHESTRATOR_H

#include <atomic>
#include <functional>
#include <string>

namespace gateway::application {

struct TripCallbacks {
  // TripInit received from backend.
  // vehicle should call queryEta(start, end) then report ETA
  std::function<void(int64_t request_id, double start_lat, double start_lon,
                     double end_lat, double end_lon)>
      on_trip_init;

  // TripMove received from backend
  // Vehicle should call autoware->move() then report status.
  std::function<void(int64_t trip_id, double lat, double lon)> on_trip_move;
};

class TripOrchestrator {
 public:
  explicit TripOrchestrator(const std::string& vin_number);

  void set_callbacks(TripCallbacks callbacks);

  /**
   * @brief Called by MqttRouter when topic/trip/init arrives.Expected payload
   * keys: vinNumber requestId startLong startLat endLong endLat
   * @param payload MQTT Payload sent.
   */
  void handle_trip_init(const std::string& payload);

  /**
   * @brief Called by MqttRouter when topic/trip/move arrives. Expected payload
   * keys: vinNumber tripId longitude latitude Stores the server-assigned
   * trip_id for all subsequent RPCs.
   * @param payload MQTT Payload sent.
   */
  void handle_trip_move(const std::string& payload);

  int64_t active_trip_id() const { return trip_id_.load(); }
  int64_t active_request_id() const { return request_id_.load(); }
  std::string vin() const { return vin_number_; }

  void set_trip_id(int64_t id) { trip_id_.store(id); }

 private:
  std::string vin_number_;
  TripCallbacks trip_callbacks_;
  std::atomic<int64_t> trip_id_{0};
  std::atomic<double> request_id_{0};
};

}  // namespace gateway::application
#endif  // VEHICLECLOUDGATEWAY_TRIP_ORCHESTRATOR_H
