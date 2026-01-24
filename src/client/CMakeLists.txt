# Client sources
add_library(client_lib OBJECT
        client.cpp
)

target_include_directories(client_lib PRIVATE
        ${CMAKE_SOURCE_DIR}/build/proto # generated vehicle_gateway.pb.h
        ${CMAKE_SOURCE_DIR}/src         # your own includes
)

target_link_libraries(client_lib PUBLIC
        proto_lib            # already built by server CMake
        gRPC::grpc++
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)
