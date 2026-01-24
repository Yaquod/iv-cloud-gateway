//
// Created by alaa-hassan on 17‏/1‏/2026.
//

#include "auth_manager.h"

#include <spdlog/spdlog.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "../constants.h"

AuthManager::AuthManager(cloud_gateway::HttpClient *http_client)
    : http_client(http_client) {
  set_headers();
}

void AuthManager::admin_setup() {
  if (cred.admin_verified) {
    spdlog::info("admin already verified");
    return;
  }

  if (login()) {
    cred.admin_verified = true;
    return;
  }

  setup_new_admin();
}

void AuthManager::setup_new_admin() {
  try {
    nlohmann::json signup_data = {{"email", cred.email},
                                  {"password", cred.password},
                                  {"firstName", cred.firstName},
                                  {"lastName", cred.lastName},
                                  {"phoneNumber", cred.phone}};

    std::map<std::string, std::string> signup_headers;
    signup_headers["Content-Type"] = "application/json";

    auto response = http_client->Post(VehicleGatewayConstants::SIGNUP_URL,
                                      signup_headers, signup_data.dump());
    auto json = nlohmann::json::parse(response.body);

    if (json["success"].get<bool>()) {
      spdlog::info("signup successfully");
    } else {
      throw std::runtime_error("Signup failed: " +
                               json["message"].get<std::string>());
    }
  } catch (std::exception &e) {
    std::string err_msg = e.what();
    if (err_msg.find("400") != std::string::npos) {
      spdlog::warn("from signup admin already exist");
    } else {
      spdlog::error("signup failed");
      throw;
    }
  }

  try {
    nlohmann::json verify_data = {{"email", cred.email}, {"code", cred.code}};

    auto response = http_client->Post(VehicleGatewayConstants::VERIFY_URL,
                                      headers, verify_data.dump());
    auto json = nlohmann::json::parse(response.body);

    if (json["success"].get<bool>()) {
      spdlog::info("verification successfully");
    } else {
      throw std::runtime_error("verification failed: " +
                               json["message"].get<std::string>());
    }

  } catch (std::exception &e) {
    spdlog::error("[SETUP] Verification failed: {}", e.what());
    throw;
  }

  try {
    login();
    cred.admin_verified = true;
  } catch (std::exception &e) {
    spdlog::error("[SETUP] Login failed: {}", e.what());
    throw;
  }
}

bool AuthManager::login() {
  try {
    nlohmann::json login_data = {{"email", cred.email},
                                 {"password", cred.password}};

    auto response = http_client->Post(VehicleGatewayConstants::LOGIN_URL,
                                      headers, login_data.dump());
    auto json = nlohmann::json::parse(response.body);

    if (json["success"].get<bool>()) {
      cred.token = json["data"]["accessToken"].get<std::string>();
      spdlog::info("login successfully");
      return true;
    }

    return false;

  } catch (std::exception &e) {
    spdlog::debug("[AUTH] Login test failed: {}", e.what());
    return false;
  }
}

void AuthManager::create_vehicles() {
  std::string token = get_valid_token();

  try {
    nlohmann::json vehicle_data = {
        {"vinNumber", data.vin_number}, {"plateNo", data.plate_no},
        {"color", data.color},          {"carCompany", data.car_company},
        {"model", data.model},          {"seats", data.seat_no}};

    headers["Authorization"] = "Bearer " + cred.token;

    auto response =
        http_client->Post(VehicleGatewayConstants::CREATE_VEHICLE_URL, headers,
                          vehicle_data.dump());
    auto json = nlohmann::json::parse(response.body);

    if (response.status_code == 201) {
      spdlog::info("Vehicle created successfully");

    } else if (response.status_code == 400) {
      spdlog::info("vehicle already exist");

    } else if (response.status_code == 401) {
      spdlog::info("unauthorized needed");
      if (login()) {
        spdlog::info("Re-authenticated successfully");
        cred.admin_verified = true;
      } else {
        setup_new_admin();
        create_vehicles();
      }
    }

  }

  catch (std::exception &e) {
    std::string err_msg = e.what();
    if (err_msg.find("already exist") != std::string::npos) {
      spdlog::info("vehicle already exist");

    } else if (err_msg.find("Authorization failed") != std::string::npos) {
      spdlog::info("Authorization failed");
    } else {
      spdlog::error("Failed to create vehicle: {}", err_msg);
    }
  }
}

std::string AuthManager::get_valid_token() {
  if (!cred.token.empty()) {
    return cred.token;
  }

  login();
  return cred.token;
}

void AuthManager::set_headers() {
  headers["Content-Type"] = "application/json";
}
