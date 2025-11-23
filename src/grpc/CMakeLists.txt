
# src/grpc_server/CMakeLists.txt
add_library(vehicle_gateway_service_lib OBJECT
        vehicle_gateway_service.cpp
        vehicle_gateway_service.h


)

# Include paths for generated gRPC files
target_include_directories(vehicle_gateway_service_lib
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/proto
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vehicle_gateway_service_lib
        PUBLIC
        proto_lib
        gRPC::grpc++
        spdlog::spdlog
)

target_compile_options(vehicle_gateway_service_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)
