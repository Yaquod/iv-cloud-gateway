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

#include <fstream>
#include <nlohmann/json.hpp>

#include "infra/constants.h"



namespace {

const std::filesystem::path kCredentialsPath =
    std::filesystem::path(CREDENTIALS_DIR) /
    "vehicle_credentials.json";

}  



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

  spdlog::info("[AuthService] login failed must signup first");

  if (!signup()) {
    spdlog::error("[AuthService] signup failed");
    return false;
  }

  if (!verify_code()) {
    spdlog::error("[AuthService] verify_code failed");
    return false;
  }

  if (login()) {
    return true;
  }

  spdlog::error("[AuthService] login failed after signup and verify_code");

  return false;
}

bool AuthService::create_vehicle(bool retry_after_login,
                                 bool retry_after_network) {

                                  
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

  // vechile created successfully = 201
  if (resp.status_code == 201) {
    spdlog::info("[AuthService] vehicle created successfully");
    vehicle_registered_ = true;

    try {
      auto j = nlohmann::json::parse(resp.body);
      vechile_api_key_ = j.at("data").at("apiKey").get<std::string>();
      vechile_api_secret_ = j.at("data").at("apiSecret").get<std::string>();
      save_vehicle_credentials();
      spdlog::info("[AuthService] captured vehicle apiKey/apiSecret");

    } catch (const std::exception& e) {
      spdlog::error("[AuthService] failed to parse apiKey/apiSecret: {}",
                    e.what());
      return false;
    }

    return true;
  }

  // vechile alreay exists = 409
  if (resp.status_code == 409) {
    spdlog::info("[AuthService] vehicle already exists");
    if (!load_vehicle_credentials()) {
      spdlog::error(
          "[AuthService] vehicle exists but credentials file not found");
      return false;
    }
    vehicle_registered_ = true;
    return true;
  }

  // admin token expired = 401
  if (resp.status_code == 401) {
    if (retry_after_login) {
      spdlog::error(
          "[AuthService] create_vehicle still returned 401 after re-login");
      return false;
    }

    spdlog::info("[AuthService] token expired — re-logging in");
    token_.clear();
    if (!login()) {
      spdlog::error("[AuthService] re-login failed — cannot register vehicle");
      return false;
    }

    // Retry once after re-login
    return create_vehicle(true);
  }

  if (!resp.success) {
    spdlog::warn(
        "[AuthService] vehicle registration failed due to connectivity: {}",
        resp.error_message);

    if (retry_after_network) {
      spdlog::error("[AuthService] network retry failed");
      return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // retry once after network failure
    return create_vehicle(retry_after_login, true);
  }

  spdlog::error("[AuthService] create_vehicle failed with status {}",
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

bool AuthService::vechile_login() {
  if (vechile_authenticated_) {
    spdlog::info("[AuthService] vehicle already authenticated");
    return true;
  }

  if (vechile_api_key_.empty() || vechile_api_secret_.empty()) {
    spdlog::info("[AuthService] no credentials");
    if (!load_vehicle_credentials()) {
      spdlog::warn(
          "[AuthService] vehicle apiKey/apiSecret not available for login");
      return false;
    }
  }

  try {
    nlohmann::json body = {{"apiKey", vechile_api_key_},
                           {"apiSecret", vechile_api_secret_}};

    auto resp =
        http_client_.Post(constants::VehicleGatewayConstants::kVehicleLoginUrl,
                          json_headers_, body.dump());

    if (!resp.success || resp.body.empty()) {
      spdlog::debug("[AuthService] vehicle login failed: {}",
                    resp.error_message);
      return false;
    }

    auto j = nlohmann::json::parse(resp.body);
    if (!j.value("success", false)) return false;

    vechile_access_token_ = j.at("data").at("accessToken").get<std::string>();
    spdlog::info("[AuthService] vehicle accessToken captured");

save_vehicle_credentials();

    vechile_authenticated_ = true;
    spdlog::info("[AuthService] vehicle login successful");
    return true;

  } catch (const std::exception& e) {
    spdlog::error("[AuthService] vehicle login exception: {}", e.what());
    return false;
  }
}

bool AuthService::verify_code() {
  try {
    nlohmann::json body = {{"email", config_.admin_email},
                           {"code", config_.verify_code}};

    auto resp =
        http_client_.Post(constants::VehicleGatewayConstants::kVerifyUrl,
                          json_headers_, body.dump());

    if (resp.status_code == 200) {
      spdlog::info("[AuthService] admin verified successfully");
      return true;
    }

    spdlog::error("[AuthService] verify_code failed with status {}: {}",
                  resp.status_code, resp.body);
    return false;

  } catch (const std::exception& e) {
    spdlog::error("[AuthService] verify_code exception: {}", e.what());
    return false;
  }
}

bool AuthService::signup() {
  try {
    nlohmann::json body = {{"email", config_.admin_email},
                           {"password", config_.admin_password},
                           {"firstName", config_.admin_first_name},
                           {"lastName", config_.admin_last_name},
                           {"phoneNumber", config_.admin_phone}};

    auto resp =
        http_client_.Post(constants::VehicleGatewayConstants::kSignupUrl,
                          json_headers_, body.dump());

    if (resp.status_code == 201) {
      spdlog::info("[AuthService] signup successful");
      return true;
    }

    if (resp.status_code == 409) {
      spdlog::info("[AuthService] admin already exists");
      return true;
    }

    spdlog::error("[AuthService] signup failed with status {}: {}",
                  resp.status_code, resp.body);
    return false;

  } catch (const std::exception& e) {
    spdlog::error("[AuthService] signup exception: {}", e.what());
    return false;
  }
}

bool AuthService::save_vehicle_credentials() {

  
  try {
    std::filesystem::path path(kCredentialsPath);
    if (path.has_parent_path()) {
      std::filesystem::create_directories(path.parent_path());
    }

    spdlog::info("Saving credentials to the file");

    nlohmann::json j = {{"apiKey", vechile_api_key_},
                        {"apiSecret", vechile_api_secret_},
                        {"accessToken", vechile_access_token_}
                      };

    std::ofstream file(kCredentialsPath);
    if (!file.is_open()) {
      spdlog::error(
          "[AuthService] cannot open credentials file for writing");
      return false;
    }

    file << j.dump(2);
    spdlog::info("[AuthService] vehicle credentials saved to the file");
    return true;

  } catch (const std::exception& e) {
    spdlog::error("[AuthService] save_vehicle_credentials exception: {}",
                  e.what());
    return false;
  }
}

bool AuthService::load_vehicle_credentials() {
  try {
    std::ifstream file(kCredentialsPath);
    if (!file.is_open()) {
      spdlog::info("[AuthService] no credentials file found "
           );
      return false;
    }

    auto j = nlohmann::json::parse(file);
    vechile_api_key_ = j.at("apiKey").get<std::string>();
    vechile_api_secret_ = j.at("apiSecret").get<std::string>();
    vechile_access_token_ =
    j.value("accessToken", "");

    spdlog::info("[AuthService] vehicle credentials loaded from the file"
);
    return true;

  } catch (const std::exception& e) {
    spdlog::warn("[AuthService] load_vehicle_credentials failed: {}", e.what());
    return false;
  }
}

}  // namespace gateway::services