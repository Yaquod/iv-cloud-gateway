add_library(mqtt_client_lib OBJECT
        mqtt_client.cpp
        mqtt_client.h
)

target_include_directories(mqtt_client_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mqtt_client_lib
        PUBLIC
        async_mqtt5
        Boost::system
        Boost::thread
        spdlog::spdlog
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_features(mqtt_client_lib PUBLIC cxx_std_20)

target_compile_options(mqtt_client_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)