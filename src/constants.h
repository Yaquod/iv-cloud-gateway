//
// Created by alaa-hassan on 13‏/12‏/2025.
//

#ifndef YAQOUD_PROJECT_MQTT_TOPICS_H
#define YAQOUD_PROJECT_MQTT_TOPICS_H

#include <string>

namespace VehicleGatewayConstants {
inline const std::string TripInit = "topic/trip/init";
inline const std::string UpdateVehicleLocation = "topic/update_location";
inline const std::string TripMove = "topic/trip/move";
inline const std::string TripArrive = "topic/trip/arrive";
inline const std::string TripStatus = "topic/trip/status";
inline const std::string TripEta = "topic/trip/eta";
inline const std::string BASE_URL = "http://localhost:8000";
inline const std::string SIGNUP_URL = BASE_URL + "/api/auth/admin/signup";
inline const std::string VERIFY_URL = BASE_URL + "/api/auth/verify-code";
inline const std::string LOGIN_URL = BASE_URL + "/api/auth/login";
inline const std::string CREATE_VEHICLE_URL = BASE_URL + "/api/vehicles";

}  // namespace VehicleGatewayConstants

#endif  // YAQOUD_PROJECT_MQTT_TOPICS_H