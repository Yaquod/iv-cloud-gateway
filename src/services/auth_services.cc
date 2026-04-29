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

#include "auth_services.h"

#include <nlohmann/json.hpp>

#include "infra/constants.h"

namespace gateway::services {

AuthService::AuthService(gateway::transport::HttpClient& http_client,
                         const Config& config)
    : http_client_(http_client), config_(config) {
  json_headers_["Content-Type"] = "application/json";
}

bool AuthService::setup() {
  if (!token_.empty()) {
    spdlog::info("[AuthService] already setup with token: {}", token_);
    return true;
  }

  if (login()) {
    return true;
  }
  return false;
}

bool AuthService::create_vehicle() {
  if (vehicle_registered_) {
    spdlog::info("[AuthService] vehicle already registered");
    return true;
  }

  if (token_.empty()) {
    spdlog::warn("[AuthService] no token — skipping vehicle registration");
    return false;
  }

  ensure_auth_headers();

  nlohmann::json body = {
      {"vinNumber", config_.vin_number}, {"plateNo", config_.plate_no},
      {"color", config_.color},          {"carCompany", config_.car_company},
      {"model", config_.model},          {"seats", config_.seat_no}};

  auto resp =
      http_client_.Post(constants::VehicleGatewayConstants::kCreateVehicleUrl,
                        auth_headers_, body.dump());

  if (resp.status_code == 201) {
    spdlog::info("[AuthService] vehicle created successfully");
    vehicle_registered_ = true;
    return true;
  }

  if (resp.status_code == 400) {
    spdlog::info("[AuthService] vehicle already exists");
    vehicle_registered_ = true;
    return true;
  }

  if (resp.status_code == 401) {
    spdlog::info("[AuthService] token expired — re-logging in");
    token_.clear();
    if (!login()) {
      spdlog::error("[AuthService] re-login failed — cannot register vehicle");
      return false;
    }
    // Retry once
    ensure_auth_headers();
    auto retry =
        http_client_.Post(constants::VehicleGatewayConstants::kCreateVehicleUrl,
                          auth_headers_, body.dump());
    if (retry.status_code == 201 || retry.status_code == 400) {
      vehicle_registered_ = true;
      return true;
    }
    spdlog::error(
        "[AuthService] vehicle registration failed after re-login: {}",
        retry.status_code);
    return false;
  }

  if (!resp.success) {
    spdlog::warn(
        "[AuthService] vehicle registration failed due to connectivity: {}",
        resp.error_message);
    return false;
  }

  spdlog::error("[AuthService] unexpected status {} from create_vehicle",
                resp.status_code);
  return false;
}

bool AuthService::login() {
  try {
    nlohmann::json body = {{"email", config_.admin_email},
                           {"password", config_.admin_password}};

    auto resp = http_client_.Post(constants::VehicleGatewayConstants::kLoginUrl,
                                  json_headers_, body.dump());

    if (!resp.success || resp.body.empty()) {
      spdlog::debug("[AuthService] login failed: {}", resp.error_message);
      return false;
    }

    auto j = nlohmann::json::parse(resp.body);
    if (!j.value("success", false)) return false;

    token_ = j.at("data").at("accessToken").get<std::string>();
    spdlog::info("[AuthService] login successful");
    return true;

  } catch (const std::exception& e) {
    spdlog::debug("[AuthService] login exception: {}", e.what());
    return false;
  }
}

void AuthService::ensure_auth_headers() {
  if (token_.empty()) {
    spdlog::warn("[AuthService] no token available for auth headers");
    return;
  }
  auth_headers_ = json_headers_;
  auth_headers_["Authorization"] = "Bearer " + token_;
}

}  // namespace gateway::services