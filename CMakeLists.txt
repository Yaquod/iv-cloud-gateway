cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

project(VehicleCloudGateway VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable address/thread sanitizers" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake")
    include(CompilerWarnings)
endif()

if(ENABLE_SANITIZERS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sanitizers.cmake")
    include(Sanitizers)
    enable_sanitizers()
endif()

message(STATUS "Finding system dependencies...")

# Threading
find_package(Threads REQUIRED)

# OpenSSL and zlib
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Boost - system is header-only
set(Boost_ROOT /usr/local)
set(Boost_NO_SYSTEM_PATHS OFF)
find_package(Boost 1.89 REQUIRED COMPONENTS thread)

# gRPC and Protobuf from system packages
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG QUIET)

if(NOT gRPC_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRPC REQUIRED grpc++)
    pkg_check_modules(GRPCPP REQUIRED grpc++>=1.16.0)

    if(NOT TARGET gRPC::grpc++)
        add_library(gRPC::grpc++ INTERFACE IMPORTED)
        set_target_properties(gRPC::grpc++ PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${GRPC_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES "${GRPC_LIBRARIES}"
        )
    endif()

    if(NOT TARGET gRPC::grpc++_reflection)
        add_library(gRPC::grpc++_reflection INTERFACE IMPORTED)
        set_target_properties(gRPC::grpc++_reflection PROPERTIES
                INTERFACE_LINK_LIBRARIES "grpc++_reflection"
        )
    endif()

    # Find grpc_cpp_plugin
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    if(NOT GRPC_CPP_PLUGIN)
        message(FATAL_ERROR "grpc_cpp_plugin not found")
    endif()

    # Create target for plugin
    add_executable(gRPC::grpc_cpp_plugin IMPORTED)
    set_target_properties(gRPC::grpc_cpp_plugin PROPERTIES
            IMPORTED_LOCATION ${GRPC_CPP_PLUGIN}
    )
endif()

# Paho MQTT (from system packages)
message(STATUS "Finding Paho MQTT libraries...")

# Find Paho MQTT C library
find_library(PAHO_MQTT_C_LIB
    NAMES paho-mqtt3as libpaho-mqtt3as
    PATHS /usr/lib64 /usr/lib /usr/local/lib
)

# Find Paho MQTT C++ library
find_library(PAHO_MQTT_CPP_LIB
    NAMES paho-mqttpp3 libpaho-mqttpp3
    PATHS /usr/lib64 /usr/lib /usr/local/lib
)

# Find Paho MQTT headers
find_path(PAHO_MQTT_INCLUDE_DIR
    NAMES mqtt/async_client.h
    PATHS /usr/include /usr/local/include
)

if(NOT PAHO_MQTT_C_LIB)
    message(FATAL_ERROR "Paho MQTT C library not found")
endif()

if(NOT PAHO_MQTT_CPP_LIB)
    message(FATAL_ERROR "Paho MQTT C++ library not found")
endif()

if(NOT PAHO_MQTT_INCLUDE_DIR)
    message(FATAL_ERROR "Paho MQTT headers not found")
endif()

message(STATUS "  Paho MQTT C:     ${PAHO_MQTT_C_LIB}")
message(STATUS "  Paho MQTT C++:   ${PAHO_MQTT_CPP_LIB}")
message(STATUS "  Paho MQTT Include: ${PAHO_MQTT_INCLUDE_DIR}")

add_library(PahoMqttC SHARED IMPORTED)
set_target_properties(PahoMqttC PROPERTIES
    IMPORTED_LOCATION ${PAHO_MQTT_C_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${PAHO_MQTT_INCLUDE_DIR}
)

add_library(PahoMqttCpp SHARED IMPORTED)
set_target_properties(PahoMqttCpp PROPERTIES
    IMPORTED_LOCATION ${PAHO_MQTT_CPP_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${PAHO_MQTT_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES PahoMqttC
)

add_library(mqtt_client INTERFACE)
target_link_libraries(mqtt_client INTERFACE
        PahoMqttCpp
        PahoMqttC
)

message(STATUS "Configuring third-party dependencies...")
add_subdirectory(third_party)

add_subdirectory(proto)

add_subdirectory(src)

add_executable(vehicle_gateway src/main.cpp)
target_link_libraries(vehicle_gateway PRIVATE gateway_lib)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET vehicle_gateway POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:vehicle_gateway>
            COMMENT "Stripping vehicle_gateway binary"
    )
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS vehicle_gateway
        RUNTIME DESTINATION bin)

install(DIRECTORY config/
        DESTINATION /etc/vehicle-gateway
        FILES_MATCHING PATTERN "*.yaml")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/systemd/vehicle-gateway.service")
    install(FILES systemd/vehicle-gateway.service
            DESTINATION /lib/systemd/system)
endif()

message(STATUS "==========================================")
message(STATUS "Vehicle Cloud Gateway Configuration")
message(STATUS "==========================================")
message(STATUS "Version:           ${PROJECT_VERSION}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "System Dependencies:")
message(STATUS "  Protobuf:        ${Protobuf_VERSION}")
message(STATUS "  gRPC:            ${gRPC_VERSION}")
message(STATUS "  OpenSSL:         ${OPENSSL_VERSION}")
message(STATUS "  Boost:           ${Boost_VERSION}")
message(STATUS "  Paho MQTT C:     ${PahoMqttC_VERSION}")
message(STATUS "  Paho MQTT C++:   ${PahoMqttCpp_VERSION}")
message(STATUS "")
message(STATUS "Third-Party (built from source):")
message(STATUS "  nlohmann/json")
message(STATUS "  spdlog")
message(STATUS "  yaml-cpp")
if(BUILD_TESTS)
    message(STATUS "  GoogleTest")
endif()
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  Tests:           ${BUILD_TESTS}")
message(STATUS "  Sanitizers:      ${ENABLE_SANITIZERS}")
message(STATUS "==========================================")