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

#include <cstdint>
#include <string>

namespace gateway {
struct Config {
  // Vehicle identity
  std::string vin_number = "ORIN_NANO_001";

  // MQTT
  std::string mqtt_broker = "localhost";
  uint16_t mqtt_port = 1883;
  std::string mqtt_client_id = "vehicle_gateway_client";

  std::string autoware_grpc_addr = "192.168.64.7:50051";

  // gRPC
  std::string grpc_listen = "192.168.64.7:50051";

  // Auth / HTTP backend
  std::string base_url = "http://localhost:8000";
  std::string admin_email = "";
  std::string admin_password = "";
  std::string admin_first_name = "";
  std::string admin_last_name = "";
  std::string admin_phone = "";
  std::string verify_code = "";

  // Vehicle registration
  std::string plate_no = "";
  std::string color = "";
  std::string car_company = "";
  std::string model = "";
  int seat_no = 4;
};
}  // namespace gateway