add_subdirectory(transport/http_client)
add_subdirectory(transport/mqtt_client)
add_subdirectory(grpc)
add_subdirectory(client)
add_subdirectory(auth_manager)

add_library(gateway_lib STATIC
        gateway.cpp
        $<TARGET_OBJECTS:http_client_lib>
        $<TARGET_OBJECTS:mqtt_client_lib>
        $<TARGET_OBJECTS:vehicle_gateway_service_lib>
        $<TARGET_OBJECTS:vehicle_gateway_auth_manager_lib>
        $<TARGET_OBJECTS:client_lib>
        infra/config.h


)

target_include_directories(gateway_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_SOURCE_DIR}/src
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gateway_lib
        PUBLIC
        proto_lib
        gRPC::grpc++_unsecure
        async_mqtt5
        Boost::thread
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        spdlog::spdlog
        yaml-cpp
        PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
)

set_target_properties(gateway_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)