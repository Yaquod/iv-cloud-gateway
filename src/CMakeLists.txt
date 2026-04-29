add_subdirectory(transport/http_client)
add_subdirectory(transport/mqtt_client)
add_subdirectory(services)
add_subdirectory(application)

add_library(gateway_lib STATIC
        infra/config.h
)

target_link_libraries(gateway_lib
        PUBLIC
        application_lib
        services_lib
        mqtt_client_lib
        http_client_lib
        proto_lib
        Boost::thread
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        spdlog::spdlog
        yaml-cpp
        PRIVATE
        Threads::Threads
)

target_include_directories(gateway_lib PUBLIC "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src")