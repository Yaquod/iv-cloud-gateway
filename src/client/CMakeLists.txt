cmake_minimum_required(VERSION 3.16)
project(vehicle_gateway_client CXX)

set(CMAKE_CXX_STANDARD 17)

# Include paths
include_directories(
        ${CMAKE_SOURCE_DIR}/build/proto   # generated vehicle_gateway.pb.h
        ${CMAKE_SOURCE_DIR}/src           # your own includes
)

# Create client library
add_library(client_lib STATIC
        client.cpp
)

target_include_directories(client_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client_lib
        PUBLIC
        proto_lib            # already built by server CMake
        gRPC::grpc++
        spdlog::spdlog
)

target_compile_options(client_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)
