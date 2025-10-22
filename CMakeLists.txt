cmake_minimum_required(VERSION 3.20)
project(VehicleCloudGateway VERSION 1.0.0 LANGUAGES CXX C)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

# find packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

# Include Module directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake")
    include(CompilerWarnings)
    set_project_warnings()
endif()

if (ENABLE_SANITIZERS)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sanitizers.cmake")
        include(Sanitizers)
        enable_sanitizers()
    endif()
endif()

# Third-party libraries
add_subdirectory(third_party)

# Protocol Buffers Generation
add_subdirectory(proto)

# Source files
add_subdirectory(src)

add_executable(vehicle_gateway src/main.cpp)
target_link_libraries(vehicle_gateway PRIVATE gateway_lib)

# Tests
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# TODO: Installation as a system-d

# Print configuration summary
message(STATUS "==============================")
message(STATUS "VehicleCloudGateway Configuration")
message(STATUS "==============================")
message(STATUS "Configuration Summary:")
message(STATUS "  Build Tests:        ${BUILD_TESTS}")
message(STATUS "  Enable Coverage:    ${ENABLE_COVERAGE}")
message(STATUS "  Enable Sanitizers:  ${ENABLE_SANITIZERS}")
message(STATUS "  C++ Standard:       C++${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenSSL Version:    ${OPENSSL_VERSION}")
message(STATUS "  ZLIB Version:       ${ZLIB_VERSION}")
message(STATUS "  spdlog Version:     ${spdlog_VERSION}")
message(STATUS "  yaml-cpp Version:   ${yaml-cpp_VERSION}")