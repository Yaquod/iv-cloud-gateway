// //
// // Created by alaa-hassan on 17‏/1‏/2026.
// //
//
// #include "auth_manager.h"
//
// #include <spdlog/spdlog.h>
//
// #include <iostream>
// #include <nlohmann/json.hpp>
//
// #include "../infra/constants.h"
//
// AuthManager::AuthManager(cloud_gateway::HttpClient *http_client)
//     : http_client(http_client) {
//   set_headers();
// }
//
// void AuthManager::admin_setup() {
//   if (cred.admin_verified) {
//     spdlog::info("admin already verified");
//     return;
//   }
//
//   if (login()) {
//     cred.admin_verified = true;
//     return;
//   }
//
//   try {
//     setup_new_admin();
//   } catch (const std::exception &e) {
//     spdlog::warn("Admin setup failed: {}. Continuing without
//     authentication...",
//                  e.what());
//
//     // Don't crash the application - continue without authentication
//     // The application can still handle MQTT communication
//   }
// }
//
// void AuthManager::setup_new_admin() {
//   try {
//     nlohmann::json signup_data = {{"email", cred.email},
//                                   {"password", cred.password},
//                                   {"firstName", cred.firstName},
//                                   {"lastName", cred.lastName},
//                                   {"phoneNumber", cred.phone}};
//
//     std::map<std::string, std::string> signup_headers;
//     signup_headers["Content-Type"] = "application/json";
//
//     auto response = http_client->Post(VehicleGatewayConstants::SIGNUP_URL,
//                                       signup_headers, signup_data.dump());
//
//     if (!response.success) {
//       throw std::runtime_error("HTTP request failed: " +
//                                response.error_message);
//     }
//
//     if (response.body.empty()) {
//       throw std::runtime_error("Received empty response from server");
//     }
//
//     auto json = nlohmann::json::parse(response.body);
//
//     if (json["success"].get<bool>()) {
//       spdlog::info("signup successfully");
//     } else {
//       throw std::runtime_error("Signup failed: " +
//                                json["message"].get<std::string>());
//     }
//   } catch (std::exception &e) {
//     std::string err_msg = e.what();
//     if (err_msg.find("400") != std::string::npos) {
//       spdlog::warn("from signup admin already exist");
//     } else {
//       spdlog::error("signup failed");
//       throw;
//     }
//   }
//
//   try {
//     nlohmann::json verify_data = {{"email", cred.email}, {"code",
//     cred.code}};
//
//     auto response = http_client->Post(VehicleGatewayConstants::VERIFY_URL,
//                                       headers, verify_data.dump());
//
//     if (!response.success) {
//       throw std::runtime_error("HTTP request failed: " +
//                                response.error_message);
//     }
//
//     if (response.body.empty()) {
//       throw std::runtime_error("Received empty response from server");
//     }
//
//     auto json = nlohmann::json::parse(response.body);
//
//     if (json["success"].get<bool>()) {
//       spdlog::info("verification successfully");
//     } else {
//       throw std::runtime_error("verification failed: " +
//                                json["message"].get<std::string>());
//     }
//
//   } catch (std::exception &e) {
//     spdlog::error("[SETUP] Verification failed: {}", e.what());
//     throw;
//   }
//
//   try {
//     login();
//     cred.admin_verified = true;
//   } catch (std::exception &e) {
//     spdlog::error("[SETUP] Login failed: {}", e.what());
//     throw;
//   }
// }
//
// bool AuthManager::login() {
//   try {
//     nlohmann::json login_data = {{"email", cred.email},
//                                  {"password", cred.password}};
//
//     auto response = http_client->Post(VehicleGatewayConstants::LOGIN_URL,
//                                       headers, login_data.dump());
//
//     if (!response.success) {
//       spdlog::debug("[AUTH] Login failed - HTTP request failed: {}",
//                     response.error_message);
//       return false;
//     }
//
//     if (response.body.empty()) {
//       spdlog::debug(
//           "[AUTH] Login failed - Received empty response from server");
//       return false;
//     }
//
//     auto json = nlohmann::json::parse(response.body);
//
//     if (json["success"].get<bool>()) {
//       cred.token = json["data"]["accessToken"].get<std::string>();
//       spdlog::info("login successfully");
//       return true;
//     }
//
//     return false;
//
//   } catch (std::exception &e) {
//     spdlog::debug("[AUTH] Login test failed: {}", e.what());
//     return false;
//   }
// }
//
// void AuthManager::create_vehicles() {
//   if (cred.vehicle_created) {
//     spdlog::info("vehicle already created");
//     return;
//   }
//
//   std::string token = get_valid_token();
//
//   if (token.empty()) {
//     spdlog::warn(
//         "No valid authentication token available. Skipping vehicle "
//         "creation...");
//     return;
//   }
//
//   try {
//     nlohmann::json vehicle_data = {
//         {"vinNumber", data.vin_number}, {"plateNo", data.plate_no},
//         {"color", data.color},          {"carCompany", data.car_company},
//         {"model", data.model},          {"seats", data.seat_no}};
//
//     headers["Authorization"] = "Bearer " + cred.token;
//
//     auto response =
//         http_client->Post(VehicleGatewayConstants::CREATE_VEHICLE_URL,
//         headers,
//                           vehicle_data.dump());
//
//     if (!response.success) {
//       throw std::runtime_error("HTTP request failed: " +
//                                response.error_message);
//     }
//
//     if (response.body.empty()) {
//       throw std::runtime_error("Received empty response from server");
//     }
//
//     auto json = nlohmann::json::parse(response.body);
//
//     if (response.status_code == 201) {
//       spdlog::info("Vehicle created successfully");
//       cred.vehicle_created = true;
//
//     } else if (response.status_code == 400) {
//       spdlog::info("vehicle already exist");
//       cred.vehicle_created = true;
//
//     } else if (response.status_code == 401) {
//       spdlog::info("unauthorized needed");
//       if (login()) {
//         spdlog::info("Re-authenticated successfully");
//         cred.admin_verified = true;
//       } else {
//         setup_new_admin();
//         create_vehicles();
//       }
//     }
//
//   } catch (const std::exception &e) {
//     std::string err_msg = e.what();
//     if (err_msg.find("already exist") != std::string::npos) {
//       spdlog::info("vehicle already exist");
//       cred.vehicle_created = true;
//     } else if (err_msg.find("Authorization failed") != std::string::npos) {
//       spdlog::info("Authorization failed");
//     } else if (err_msg.find("HTTP request failed") != std::string::npos) {
//       spdlog::warn(
//           "Vehicle creation failed due to connectivity issues: {}. Continuing
//           " "without vehicle registration...", err_msg);
//       // Don't crash - continue without vehicle registration
//     } else {
//       spdlog::error("Failed to create vehicle: {}", err_msg);
//     }
//   }
// }
//
// std::string AuthManager::get_valid_token() {
//   if (!cred.token.empty()) {
//     return cred.token;
//   }
//
//   if (login()) {
//     return cred.token;
//   }
//
//   // Return empty token if login fails - let the caller handle this
//   spdlog::warn(
//       "Failed to obtain valid token - authentication may be unavailable");
//   return "";
// }
//
// void AuthManager::set_headers() {
//   headers["Content-Type"] = "application/json";
// }
