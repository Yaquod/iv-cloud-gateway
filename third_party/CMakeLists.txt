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

# GoogleTest (for unit tests)
add_subdirectory(googletest)