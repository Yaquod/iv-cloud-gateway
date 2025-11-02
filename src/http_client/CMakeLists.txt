add_library(http_client_lib OBJECT
        http_client.cpp
        http_client.h
)

target_include_directories(http_client_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(http_client_lib
        PUBLIC
        Boost::beast
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
        spdlog::spdlog
)

target_compile_options(http_client_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)