cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(VehicleCloudGateway VERSION 1.0.0 LANGUAGES CXX C)

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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SubmoduleHelpers.cmake")
    include(SubmoduleHelpers)
endif()

if(ENABLE_SANITIZERS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sanitizers.cmake")
    include(Sanitizers)
    enable_sanitizers()
endif()

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)

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

# Install systemd service file only on Linux.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/systemd/vehicle-gateway.service")
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