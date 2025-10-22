add_subdirectory(http_client)
add_subdirectory(mqtt_client)

add_library(gateway_lib STATIC
        gateway.cpp
        $<TARGET_OBJECTS:http_client>
        $<TARGET_OBJECTS:mqtt_client>
)

target_include_directories(gateway_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gateway_lib
        PUBLIC
        grpc_cpp
        mqtt
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
        spdlog::spdlog
        yaml-cpp
        proto_lib
        PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
)

set_target_properties(gateway_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)