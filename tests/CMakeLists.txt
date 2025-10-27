file(GLOB_RECURSE UNIT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/*_test.cpp")

add_executable(gateway_tests
        test_main.cpp
        ${UNIT_TEST_SOURCES}
        unit/mqtt_test.cpp
)

target_include_directories(gateway_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests/mocks
)

target_link_libraries(gateway_tests
        PRIVATE
        gateway_lib
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(gateway_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PROPERTIES
        TIMEOUT 60
)

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
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
    )

    gtest_discover_tests(gateway_integration_tests
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()