set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

message(STATUS "Building third-party dependencies...")

# ============================================================================
# async_mqtt5 (header-only MQTT5 client)
# ============================================================================
message(STATUS "  - async_mqtt5 (header-only MQTT5)")
add_library(async_mqtt5 INTERFACE)
target_include_directories(async_mqtt5 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/async-mqtt5/include
)
target_compile_features(async_mqtt5 INTERFACE cxx_std_17)

target_link_libraries(async_mqtt5 INTERFACE
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
)
# ============================================================================
# nlohmann/json (header-only)
set(JSON_Install OFF CACHE BOOL "Do not install nlohmann_json")
add_subdirectory(json)

set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as a static library")
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use bundled fmt library")
add_subdirectory(spdlog)

# yaml-cpp
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Do not build yaml-cpp tests")
add_subdirectory(yaml-cpp)

if(VGW_BUILD_PAHO_FROM_SOURCE)
    # Paho MQTT C Library
    set(PAHO_INSTALL_LIBRARIES OFF CACHE BOOL "Do not install Paho C libraries") # For paho.mqtt.c
    set(PAHO_WITH_SSL ON CACHE BOOL "Enable SSL for Paho C")
    set(PAHO_BUILD_STATIC ON CACHE BOOL "Build Paho C as a static library")
    set(PAHO_BUILD_SAMPLES OFF CACHE BOOL "Do not build Paho C samples")
    add_subdirectory(paho.mqtt.c)

    # Paho MQTT C++ Library
    vgw_add_subdirectory_no_install(paho.mqtt.cpp)
endif()

# GoogleTest (for unit tests)
add_subdirectory(googletest)