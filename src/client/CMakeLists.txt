cmake_minimum_required(VERSION 3.16)

project(vehicle_gateway_client CXX)

set(CMAKE_CXX_STANDARD 17)

# Include paths
include_directories(
        ${CMAKE_SOURCE_DIR}/build/proto       # generated vehicle_gateway.pb.h
        ${CMAKE_SOURCE_DIR}/src              # your own includes
)

# Client sources
add_executable(vehicle_client
        client.cpp
        main.cpp
)

# Link with the already existing proto_lib + grpc
target_link_libraries(vehicle_client
        proto_lib            # already built by server CMake
        gRPC::grpc++
        spdlog::spdlog
)
