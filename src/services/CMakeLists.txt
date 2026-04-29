add_library(services_lib OBJECT
        rpc_dispatcher.cc
        rpc_dispatcher.h
        mqtt_router.cc
        mqtt_router.h
        grpc_server.cc
        grpc_server.h
        auth_services.h
        auth_services.cc
        vehicle_stream_handler.h
        stream_tag.h
        vehicle_stream_handler.cc
)

target_include_directories(services_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE .
)

target_link_libraries(services_lib
        PUBLIC
        proto_lib
        Boost::thread
        gRPC::grpc++
        OpenSSL::SSL
        OpenSSL::Crypto
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

target_compile_options(services_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)