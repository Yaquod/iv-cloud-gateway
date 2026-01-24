cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project(VehicleCloudGateway VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SubmoduleHelpers.cmake")
    include(SubmoduleHelpers)
endif()

if(ENABLE_SANITIZERS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sanitizers.cmake")
    include(Sanitizers)
    enable_sanitizers()
endif()

# On macOS, we need to hint to CMake where to find libraries installed via Homebrew.
if(APPLE)
    if(DEFINED ENV{HOMEBREW_PREFIX})
        set(HOMEBREW_PREFIX "$ENV{HOMEBREW_PREFIX}")
    elseif(IS_DIRECTORY "/opt/homebrew")
        set(HOMEBREW_PREFIX "/opt/homebrew") # Apple Silicon
    elseif(IS_DIRECTORY "/usr/local")
        set(HOMEBREW_PREFIX "/usr/local") # Intel
    endif()

    # If Homebrew is found, add its paths to CMake's search prefixes.
    if(HOMEBREW_PREFIX AND IS_DIRECTORY "${HOMEBREW_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
        set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
endif()

message(STATUS "Finding system dependencies...")

# Threading
find_package(Threads REQUIRED)

# OpenSSL and zlib
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

if(APPLE)
    set(Boost_NO_BOOST_CMAKE ON)
    set(Boost_NO_WARN ON)
endif()
find_package(Boost 1.74.0 QUIET COMPONENTS thread)

if(NOT Boost_FOUND AND APPLE AND HOMEBREW_PREFIX)
    message(STATUS "Boost not found via find_package. Attempting manual discovery on macOS.")
    set(Boost_NO_BOOST_LIBS ON)

    find_path(Boost_INCLUDE_DIR NAMES boost/version.hpp PATHS "${HOMEBREW_PREFIX}/include")
    find_library(Boost_THREAD_LIBRARY NAMES boost_thread PATHS "${HOMEBREW_PREFIX}/lib")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Boost
        REQUIRED_VARS Boost_INCLUDE_DIR Boost_THREAD_LIBRARY
        VERSION_VAR Boost_VERSION)

    if(Boost_FOUND)
        set(Boost_LIBRARIES ${Boost_THREAD_LIBRARY})
        set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
        add_library(Boost::thread UNKNOWN IMPORTED)
        set_target_properties(Boost::thread PROPERTIES
            IMPORTED_LOCATION "${Boost_THREAD_LIBRARY}")
    endif()
endif()

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost library. Please ensure it is installed correctly.")
endif()

find_package(gRPC REQUIRED)

if(NOT TARGET protobuf::libprotobuf AND Protobuf_LIBRARIES)
    message(STATUS "Protobuf target 'protobuf::libprotobuf' not found. Creating ALIAS.")
    add_library(protobuf::libprotobuf ALIAS ${Protobuf_LIBRARIES})
endif()

if(NOT TARGET gRPC::grpc_cpp_plugin)
    message(FATAL_ERROR "gRPC C++ plugin target (gRPC::grpc_cpp_plugin) not found. Check your gRPC installation.")
endif()

# --- Paho MQTT Dependency Handling ---
# Create an interface library to represent the Paho MQTT dependency.
add_library(vgw_paho_mqtt_iface INTERFACE)

# First, try to find Paho MQTT on the system using pkg-config. This is ideal for CI.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_PAHO_MQTT_C QUIET paho-mqtt3as)
    pkg_check_modules(PC_PAHO_MQTT_CPP QUIET paho-mqttpp3)
endif()

# If both system libraries were found, link them to our interface target.
if(PC_PAHO_MQTT_C_FOUND AND PC_PAHO_MQTT_CPP_FOUND)
    message(STATUS "Found Paho MQTT via pkg-config, will use system libraries.")
    target_link_libraries(vgw_paho_mqtt_iface INTERFACE PkgConfig::PC_PAHO_MQTT_CPP)
else()
    # Otherwise, set an option to build them from the third_party submodule.
    message(STATUS "Paho MQTT not found on system. Falling back to build from source.")
    set(VGW_BUILD_PAHO_FROM_SOURCE ON CACHE INTERNAL "Build Paho MQTT from submodule source")
endif()

message(STATUS "Configuring third-party dependencies...")
add_subdirectory(third_party)

# If we built Paho from source, we must link it to our interface library *after*
if(VGW_BUILD_PAHO_FROM_SOURCE)
    target_link_libraries(vgw_paho_mqtt_iface INTERFACE
        $<TARGET_FILE:paho-mqttpp3-static>)
endif()

add_subdirectory(proto)

add_subdirectory(src)

add_executable(vehicle_gateway src/main.cpp)
target_link_libraries(vehicle_gateway PRIVATE gateway_lib vgw_paho_mqtt_iface)

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
if(VGW_BUILD_PAHO_FROM_SOURCE)
    message(STATUS "  paho-mqtt-c (from source)")
    message(STATUS "  paho-mqtt-cpp (from source)")
endif()
if(BUILD_TESTS)
    message(STATUS "  GoogleTest")
endif()
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  Tests:           ${BUILD_TESTS}")
message(STATUS "  Sanitizers:      ${ENABLE_SANITIZERS}")
message(STATUS "==========================================")