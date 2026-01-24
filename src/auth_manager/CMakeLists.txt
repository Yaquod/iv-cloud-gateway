
# src/grpc_server/CMakeLists.txt
add_library(vehicle_gateway_auth_manager_lib OBJECT
        auth_manager.cpp
        auth_manager.h


)

# Include paths for generated gRPC files
target_include_directories(vehicle_gateway_auth_manager_lib
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/proto
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vehicle_gateway_auth_manager_lib
        PUBLIC
        proto_lib
        gRPC::grpc++
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

target_compile_options(vehicle_gateway_service_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)
