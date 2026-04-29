add_library(application_lib OBJECT
    trip_orchestrator.cc
    trip_orchestrator.h
)

target_include_directories(application_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PRIVATE .
)

target_link_libraries(application_lib
        PUBLIC
        proto_lib
        services_lib
)

target_compile_options(application_lib PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)