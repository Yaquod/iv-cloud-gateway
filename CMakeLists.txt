cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
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

if(ENABLE_SANITIZERS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sanitizers.cmake")
    include(Sanitizers)
    enable_sanitizers()
endif()

# On macOS
if(APPLE)
    if(DEFINED ENV{HOMEBREW_PREFIX})
        set(HOMEBREW_PREFIX "$ENV{HOMEBREW_PREFIX}")
    elseif(IS_DIRECTORY "/opt/homebrew")
        set(HOMEBREW_PREFIX "/opt/homebrew") # Apple Silicon
    elseif(IS_DIRECTORY "/usr/local")
        set(HOMEBREW_PREFIX "/usr/local") # Intel
    endif()

    if(HOMEBREW_PREFIX AND IS_DIRECTORY "${HOMEBREW_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
        set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
endif()

message(STATUS "Finding system dependencies...")

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

find_package(Boost 1.74.0 QUIET CONFIG COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(STATUS "Boost config discovery failed, falling back to FindBoost.cmake module.")
    find_package(Boost 1.74.0 REQUIRED COMPONENTS system thread)
endif()

if(Boost_FOUND)
    if(NOT TARGET Boost::system)
        message(STATUS "Creating Boost::system alias for compatibility.")
        add_library(Boost::system INTERFACE IMPORTED)
        target_link_libraries(Boost::system INTERFACE ${Boost_SYSTEM_LIBRARIES})
    endif()
    if(NOT TARGET Boost::thread)
        message(STATUS "Creating Boost::thread alias for compatibility.")
        add_library(Boost::thread INTERFACE IMPORTED)
        target_link_libraries(Boost::thread INTERFACE ${Boost_THREAD_LIBRARIES})
    endif()
endif()


find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    message(STATUS "Protobuf CMake config not found, falling back to PkgConfig.")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Protobuf REQUIRED IMPORTED_TARGET protobuf)
    if(NOT Protobuf_FOUND)
        message(FATAL_ERROR "Could not find Protobuf using either CMake config or pkg-config.")
    endif()
endif()


find_package(gRPC CONFIG QUIET)
if(NOT gRPC_FOUND)
    message(STATUS "gRPC config not found, falling back to PkgConfig to find grpc++.")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PC_GRPC REQUIRED IMPORTED_TARGET grpc++)
    
    if(TARGET PkgConfig::PC_GRPC)
        message(STATUS "Found gRPC via pkg-config.")
        # Create an alias for the target found by pkg-config for consistency.
        add_library(gRPC::grpc++ ALIAS PkgConfig::PC_GRPC)
    else()
        message(FATAL_ERROR "Could not find gRPC using either CMake config or pkg-config. Please try: brew reinstall grpc")
    endif()
else()
    message(STATUS "Found gRPC via CMake config.")
endif()

if(NOT TARGET gRPC::grpc_cpp_plugin)
    message(FATAL_ERROR "gRPC C++ plugin target (gRPC::grpc_cpp_plugin) not found. Check your gRPC installation.")
endif()

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

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS vehicle_gateway RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION /etc/vehicle-gateway FILES_MATCHING PATTERN "*.yaml")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/systemd/vehicle-gateway.service")
    install(FILES systemd/vehicle-gateway.service DESTINATION /lib/systemd/system)
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
message(STATUS "  OpenSSL:         ${OPENSSL_VERSION}")
message(STATUS "  Boost:           ${Boost_VERSION}")
message(STATUS "")
message(STATUS "Third-Party (built from source):")
message(STATUS "  async_mqtt5 (MQTT5 client)")
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