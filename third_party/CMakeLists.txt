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
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
)
# ============================================================================
# nlohmann/json (header-only)
# ============================================================================
message(STATUS "  - nlohmann/json (header-only)")
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
add_subdirectory(json EXCLUDE_FROM_ALL)

# ============================================================================
# spdlog
# ============================================================================
message(STATUS "  - spdlog")
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# ============================================================================
# yaml-cpp
# ============================================================================
message(STATUS "  - yaml-cpp")
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)

# ============================================================================
# Google Test (only if building tests)
# ============================================================================
if(BUILD_TESTS)
    message(STATUS "  - GoogleTest")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif()

message(STATUS "Third-party dependencies configured successfully")