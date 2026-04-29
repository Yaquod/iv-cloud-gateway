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

#include "trip_orchestrator.h"

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

namespace gateway::application {

TripOrchestrator::TripOrchestrator(const std::string& vin_number)
    : vin_number_(vin_number) {}

void TripOrchestrator::set_callbacks(TripCallbacks callbacks) {
  trip_callbacks_ = std::move(callbacks);
}

void TripOrchestrator::handle_trip_init(const std::string& payload) {
  auto j = nlohmann::json::parse(payload);

  auto vin = j.at("vinNumber").get<std::string>();
  auto request_id = j.at("requestId").get<int64_t>();
  auto start_lon = j.at("startLong").get<double>();
  auto start_lat = j.at("startLat").get<double>();
  auto end_lon = j.at("endLong").get<double>();
  auto end_lat = j.at("endLat").get<double>();

  request_id_.store(request_id);

  spdlog::info(
      "[Trip] TripInit — vin={} reqId={} start=({:.6f},{:.6f}) "
      "end=({:.6f},{:.6f})",
      vin, request_id, start_lat, start_lon, end_lat, end_lon);

  if (trip_callbacks_.on_trip_init) {
    trip_callbacks_.on_trip_init(request_id, start_lat, start_lon, end_lat,
                                 end_lon);
  }
}

void TripOrchestrator::handle_trip_move(const std::string& payload) {
  auto j = nlohmann::json::parse(payload);

  auto vin = j.at("vinNumber").get<std::string>();
  auto trip_id = j.at("tripId").get<int64_t>();
  auto lon = j.at("longitude").get<double>();
  auto lat = j.at("latitude").get<double>();

  // Store trip_id immediately, every RPC after this uses it
  trip_id_.store(trip_id);

  spdlog::info("[Trip] TripMove — vin={} tripId={} pos=({:.6f},{:.6f})", vin,
               trip_id, lat, lon);

  if (trip_callbacks_.on_trip_move) {
    trip_callbacks_.on_trip_move(trip_id, lat, lon);
  }
}

}  // namespace gateway::application