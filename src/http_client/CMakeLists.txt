add_library(http_client OBJECT
        http_client.cpp
        http_client.h
)

target_include_directories(http_client
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(http_client
        PUBLIC
        boost
        OpenSSL::SSL
        OpenSSL::Crypto
        spdlog::spdlog
)