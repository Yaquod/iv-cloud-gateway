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

#include <gtest/gtest.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>

#include "../../src/application/trip_orchestrator.h"

using namespace gateway::application;

TEST(TripOrchestratorTest, HandlesTripInitMqttPayload) {
  std::string vin = "VIN123";
  TripOrchestrator orchestrator(vin);

  std::mutex mtx;
  std::condition_variable cv;
  bool callback_called = false;
  int64_t cb_request_id = 0;
  double cb_start_lat = 0, cb_start_lon = 0, cb_end_lat = 0, cb_end_lon = 0;

  TripCallbacks callbacks;
  callbacks.on_trip_init = [&](int64_t request_id, double start_lat,
                               double start_lon, double end_lat,
                               double end_lon) {
    std::lock_guard<std::mutex> lock(mtx);
    callback_called = true;
    cb_request_id = request_id;
    cb_start_lat = start_lat;
    cb_start_lon = start_lon;
    cb_end_lat = end_lat;
    cb_end_lon = end_lon;
    cv.notify_one();
  };
  orchestrator.set_callbacks(callbacks);

  // Simulate the MQTT payload
  nlohmann::json payload_json = {{"vinNumber", "VIN123"},
                                 {"requestId", 1},
                                 {"startingLong", 35.69247052781142},
                                 {"startingLat", 139.69333226423214},
                                 {"endingLong", 35.68814679007944},
                                 {"endingLat", 139.69440756809428}};
  // The orchestrator expects keys: startLong, startLat, endLong, endLat
  // Map the incoming keys to expected ones
  nlohmann::json orchestrator_payload = {
      {"vinNumber", payload_json["vinNumber"]},
      {"requestId", payload_json["requestId"]},
      {"startLong", payload_json["startingLong"]},
      {"startLat", payload_json["startingLat"]},
      {"endLong", payload_json["endingLong"]},
      {"endLat", payload_json["endingLat"]}};

  std::string payload_str = orchestrator_payload.dump();
  orchestrator.handle_trip_init(payload_str);

  // Wait for callback
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait_for(lock, std::chrono::seconds(1), [&] { return callback_called; });
  EXPECT_TRUE(callback_called);
  EXPECT_EQ(cb_request_id, 1);
  EXPECT_DOUBLE_EQ(cb_start_lat, 139.69333226423214);
  EXPECT_DOUBLE_EQ(cb_start_lon, 35.69247052781142);
  EXPECT_DOUBLE_EQ(cb_end_lat, 139.69440756809428);
  EXPECT_DOUBLE_EQ(cb_end_lon, 35.68814679007944);
}
