//
// Created by alaa-hassan on 13‏/12‏/2025.
//

#ifndef YAQOUD_PROJECT_MQTT_TOPICS_H
#define YAQOUD_PROJECT_MQTT_TOPICS_H


#include  <string>

namespace MqttTopics {
    inline const std::string TripInit = "topic/trip/init";
    inline const std::string TripMove = "topic/trip/move";
    inline const std::string TripArrive = "topic/trip/arrive";
    inline const std::string TripStatus = "topic/trip/status";
    inline const std::string TripEta = "topic/trip/eta";
    inline const std::string URL = "api/vechile/login";



}

#endif //YAQOUD_PROJECT_MQTT_TOPICS_H