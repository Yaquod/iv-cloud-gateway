//
// Created by alaa-hassan on 17‏/1‏/2026.
//

#ifndef YAQOUD_PROJECT_AUTH_MANAGER_H
#define YAQOUD_PROJECT_AUTH_MANAGER_H
#include <string>

#include "http_client/http_client.h"

class AuthManager {
 public:
  struct Credentials {
    std::string email = "gateway@system.com";
    std::string password = "gateway123";
    std::string base_url = "http://localhost:8000";
    std::string firstName = "Gateway";
    std::string lastName = "System";
    std::string code = "111111";
    std::string phone = "0000000000";
    std::string token;
    std::string time_expired;

    bool admin_verified = false;
    bool vehicle_created = false;
  };

  struct VehicleData {
    std::string vin_number = "vin123";
    std::string plate_no = "plateNo123";
    std::string color = "white";
    std::string car_company = "Toyota";
    std::string model = "Camry";
    int seat_no = 4;
  };

  AuthManager(cloud_gateway::HttpClient* http_client);
  void admin_setup();
  void create_vehicles();
  std::string get_valid_token();
  void set_headers();

  const Credentials& get_credentials() const { return cred; }

 private:
  cloud_gateway::HttpClient* http_client;
  Credentials cred;
  VehicleData data;
  void setup_new_admin();
  bool login();
  std::map<std::string, std::string> headers;

  // time later
};
#endif  // YAQOUD_PROJECT_AUTH_MANAGER_H