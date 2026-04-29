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

#ifndef VEHICLECLOUDGATEWAY_AUTH_SERVICES_H
#define VEHICLECLOUDGATEWAY_AUTH_SERVICES_H

#include <infra/config.h>
#include <spdlog/spdlog.h>
#include <transport/http_client/http_client.h>

namespace gateway::services {
class AuthService {
 public:
  explicit AuthService(gateway::transport::HttpClient& http_client,
                       const Config& config);
  bool setup();
  bool create_vehicle();

 private:
  gateway::transport::HttpClient& http_client_;
  const Config& config_;
  std::string token_;
  bool vehicle_registered_{false};

  std::map<std::string, std::string> json_headers_;
  std::map<std::string, std::string> auth_headers_;

  bool login();
  bool signup_verify();
  void ensure_auth_headers();
};
}  // namespace gateway::services
#endif  // VEHICLECLOUDGATEWAY_AUTH_SERVICES_H
