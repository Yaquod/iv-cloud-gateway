set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

message(STATUS "Building third-party dependencies...")

# ============================================================================
# Boost
# ============================================================================
message(STATUS "  - Boost 1.89.0")

set(BOOST_INCLUDE_LIBRARIES
        system
        thread
        chrono
        date_time
        context
        CACHE STRING "" FORCE
)

set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)
add_subdirectory(boost EXCLUDE_FROM_ALL)

# ============================================================================
# nlohmann/json (header-only)
# ============================================================================
message(STATUS "  - nlohmann/json (header-only)")
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
add_subdirectory(json EXCLUDE_FROM_ALL)

# ============================================================================
# spdlog
# ============================================================================
message(STATUS "  - spdlog")
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# ============================================================================
# yaml-cpp
# ============================================================================
message(STATUS "  - yaml-cpp")
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)

# ============================================================================
# Google Test (only if building tests)
# ============================================================================
if(BUILD_TESTS)
    message(STATUS "  - GoogleTest")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif()

# ============================================================================
# Boost header-only wrapper libraries
# ============================================================================

# Boost::asio wrapper
add_library(boost_asio_wrapper INTERFACE)
target_include_directories(boost_asio_wrapper INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/asio/include
)
target_link_libraries(boost_asio_wrapper INTERFACE
        Boost::system
        Boost::thread
        Boost::date_time
        Boost::context
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)
add_library(Boost::asio ALIAS boost_asio_wrapper)

# Boost::beast wrapper
add_library(boost_beast_wrapper INTERFACE)
target_include_directories(boost_beast_wrapper INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/beast/include
)
target_link_libraries(boost_beast_wrapper INTERFACE
        Boost::asio
        Boost::system
)
add_library(Boost::beast ALIAS boost_beast_wrapper)

message(STATUS "Third-party dependencies configured successfully")