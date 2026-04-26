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
inline const std::string kTopicUpdateLocation = "topic/update_location";

inline const std::string kBaseUrl = "http://localhost:8000";
inline const std::string kSignupUrl = kBaseUrl + "/api/auth/admin/signup";
inline const std::string kVerifyUrl = kBaseUrl + "/api/auth/verify-code";
inline const std::string kLoginUrl = kBaseUrl + "/api/auth/login";
inline const std::string kCreateVehicleUrl = kBaseUrl + "/api/vehicles";

inline const std::string kGrpcListenAddr = "0.0.0.0:50051";

inline const std::string kMqttBroker = "localhost";
inline const uint16_t kMqttPort = 1883;
inline const std::string kMqttClientId = "vehicle_gateway_client";

}  // namespace VehicleGatewayConstants

}  // namespace gateway::constants