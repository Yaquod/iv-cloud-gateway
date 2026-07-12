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

#pragma once

#include <string>

namespace gateway::constants {

namespace VehicleGatewayConstants {

inline const std::string kTopicTripInit = "topic/trip/init";
inline const std::string kTopicTripMove = "topic/trip/move";
inline const std::string kTopicTripEta = "topic/trip/eta";
inline const std::string kTopicTripStatus = "topic/trip/status";
inline const std::string kTopicTripArrive = "topic/trip/arrive";
inline const std::string kTopicUpdateLocation = "topic/vehicle/update/location";
inline const std::string kTopicTripPark = "topic/trip/park";
inline const std::string kTopicOrderUpdateLocation =
    "topic/update_location/order";
inline const std::string kTopicOrderUpdateStatus = "topic/update_status/order";

inline const std::string kTopicTripCancel = "topic/trip/cancel";
inline const std::string kTopicStreamLocation = "topic/trip/stream_location";

// Deployment config (backend base URL + auth endpoints, MQTT broker/port/client,
// gRPC listen address) is no longer hardcoded here — it comes from the
// environment / .env via gateway::Config::from_env(). Auth request paths are
// built from Config::base_url in AuthService. See .env.example.

}  // namespace VehicleGatewayConstants

}  // namespace gateway::constants