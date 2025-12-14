add_subdirectory(http_client)
add_subdirectory(mqtt_client)
#added
add_subdirectory(grpc)
#added
add_subdirectory(client)

add_library(gateway_lib STATIC
        gateway.cpp
        $<TARGET_OBJECTS:http_client_lib>
        $<TARGET_OBJECTS:mqtt_client_lib>
        #added
        $<TARGET_OBJECTS:vehicle_gateway_service_lib>

)

target_include_directories(gateway_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        #added
        ${CMAKE_SOURCE_DIR}/src
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gateway_lib
        PUBLIC
        proto_lib
        #added
        client_lib
        gRPC::grpc++
        Boost::system
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

target_compile_options(gateway_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)