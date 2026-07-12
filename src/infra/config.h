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
#include <cstdlib>
#include <fstream>
#include <string>

namespace gateway {
struct Config {
  // Vehicle identity
  std::string vin_number = "ORIN_NANO_001";

  // MQTT
  std::string mqtt_broker = "localhost";
  uint16_t mqtt_port = 1883;
  std::string mqtt_client_id = "vehicle_gateway";

  std::string autoware_grpc_addr = "127.0.0.1:50051";

  // gRPC
  std::string grpc_listen = "0.0.0.0:50051";

  // Zenoh subscriber connect endpoint (autoware-agent)
  std::string zenoh_connect = "udp/127.0.0.1:7447";

  // Auth / HTTP backend
  std::string base_url = "https://yaquod.duckdns.org";
  std::string admin_email;
  std::string admin_password;
  std::string admin_first_name;
  std::string admin_last_name;
  std::string admin_phone;
  std::string verify_code;

  // Vehicle registration
  std::string plate_no;
  std::string color;
  std::string car_company;
  std::string model;
  int seat_no = 4;

  // Vehicle fixed start
  double start_lat = 35.68677918;
  double start_lon = 139.69154336;

  // Build the config from environment variables (optionally seeded from a .env
  // file) so the gateway is configured for deployment without recompiling.
  static Config from_env();
};

namespace config_detail {

// Load KEY=VALUE lines from a .env file into the process environment. Existing
// environment variables win (setenv overwrite=0), so container `-e` / compose
// env always overrides the file. A missing file is a no-op.
inline void load_dotenv(const std::string& path) {
  std::ifstream f(path);
  if (!f.is_open()) return;
  std::string line;
  while (std::getline(f, line)) {
    const auto begin = line.find_first_not_of(" \t");
    if (begin == std::string::npos) continue;
    if (line[begin] == '#') continue;
    const auto eq = line.find('=', begin);
    if (eq == std::string::npos) continue;
    std::string key = line.substr(begin, eq - begin);
    std::string val = line.substr(eq + 1);
    while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
      key.pop_back();
    while (!val.empty() && (val.back() == '\r' || val.back() == '\n' ||
                            val.back() == ' ' || val.back() == '\t'))
      val.pop_back();
    if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') ||
                            (val.front() == '\'' && val.back() == '\'')))
      val = val.substr(1, val.size() - 2);
    if (!key.empty()) setenv(key.c_str(), val.c_str(), 0);
  }
}

inline std::string env_or(const char* key, const std::string& def) {
  const char* v = std::getenv(key);
  return (v && *v) ? std::string(v) : def;
}

inline int env_int(const char* key, int def) {
  const char* v = std::getenv(key);
  if (!v || !*v) return def;
  try {
    return std::stoi(v);
  } catch (...) {
    return def;
  }
}

inline double env_double(const char* key, double def) {
  const char* v = std::getenv(key);
  if (!v || !*v) return def;
  try {
    return std::stod(v);
  } catch (...) {
    return def;
  }
}

}  // namespace config_detail

inline Config Config::from_env() {
  using namespace config_detail;
  load_dotenv(env_or("GATEWAY_ENV_FILE", ".env"));

  Config c;
  c.vin_number = env_or("VIN_NUMBER", c.vin_number);

  c.mqtt_broker = env_or("MQTT_BROKER", c.mqtt_broker);
  c.mqtt_port = static_cast<uint16_t>(env_int("MQTT_PORT", c.mqtt_port));
  c.mqtt_client_id = env_or("MQTT_CLIENT_ID", c.mqtt_client_id);

  c.autoware_grpc_addr = env_or("AUTOWARE_GRPC_ADDR", c.autoware_grpc_addr);
  c.grpc_listen = env_or("GRPC_LISTEN", c.grpc_listen);
  c.zenoh_connect = env_or("ZENOH_CONNECT", c.zenoh_connect);

  c.base_url = env_or("BASE_URL", c.base_url);
  c.admin_email = env_or("ADMIN_EMAIL", c.admin_email);
  c.admin_password = env_or("ADMIN_PASSWORD", c.admin_password);
  c.admin_first_name = env_or("ADMIN_FIRST_NAME", c.admin_first_name);
  c.admin_last_name = env_or("ADMIN_LAST_NAME", c.admin_last_name);
  c.admin_phone = env_or("ADMIN_PHONE", c.admin_phone);
  c.verify_code = env_or("VERIFY_CODE", c.verify_code);

  c.plate_no = env_or("PLATE_NO", c.plate_no);
  c.color = env_or("COLOR", c.color);
  c.car_company = env_or("CAR_COMPANY", c.car_company);
  c.model = env_or("MODEL", c.model);
  c.seat_no = env_int("SEAT_NO", c.seat_no);

  c.start_lat = env_double("START_LAT", c.start_lat);
  c.start_lon = env_double("START_LON", c.start_lon);
  return c;
}
}  // namespace gateway
