cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
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