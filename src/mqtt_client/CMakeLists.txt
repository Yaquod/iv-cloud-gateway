add_library(mqtt_client OBJECT
    mqtt_client.cpp
    mqtt_client.h
)

target_include_directories(mqtt_client
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mqtt_client
    PUBLIC
        mqtt
        spdlog::spdlog
)