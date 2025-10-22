file(GLOB_RECURSE UNIT_TEST_SOURCES "unit/**/*_test.cpp")

add_executable(gateway_unit_tests
        test_main.cpp
        ${UNIT_TEST_SOURCES}
)

target_include_directories(gateway_unit_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests/mocks
)

target_link_libraries(gateway_unit_tests
        PRIVATE
        gateway_lib
        gtest
        gmock
        gtest_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(gateway_unit_tests)

# Integration Tests
if(BUILD_INTEGRATION_TESTS)
    file(GLOB_RECURSE INTEGRATION_TEST_SOURCES "integration/**/*_test.cpp")

    add_executable(gateway_integration_tests
            test_main.cpp
            ${INTEGRATION_TEST_SOURCES}
    )

    target_include_directories(gateway_integration_tests
            PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(gateway_integration_tests
            PRIVATE
            gateway_lib
            gtest
            gmock
            gtest_main
    )

    gtest_discover_tests(gateway_integration_tests)
endif()