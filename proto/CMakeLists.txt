set(Protobuf_PROTOC_EXECUTABLE ${CMAKE_BINARY_DIR}/third_party/protobuf/protoc)
set(GRPC_CPP_PLUGIN ${CMAKE_BINARY_DIR}/third_party/grpc/grpc_cpp_plugin)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

if(PROTO_FILES)
    message(STATUS "Found ${CMAKE_CURRENT_LIST_LENGTH} .proto files")

    # Generate C++ sources from proto files
    set(PROTO_SRCS)
    set(PROTO_HDRS)
    set(GRPC_SRCS)
    set(GRPC_HDRS)

    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
        get_filename_component(PROTO_ABS ${PROTO_FILE} ABSOLUTE)

        # Output files
        set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
        set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
        set(GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
        set(GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

        list(APPEND PROTO_SRCS ${PROTO_SRC})
        list(APPEND PROTO_HDRS ${PROTO_HDR})
        list(APPEND GRPC_SRCS ${GRPC_SRC})
        list(APPEND GRPC_HDRS ${GRPC_HDR})

        add_custom_command(
                OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${GRPC_SRC} ${GRPC_HDR}
                COMMAND ${Protobuf_PROTOC_EXECUTABLE}
                ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                ${PROTO_ABS}
                DEPENDS ${PROTO_ABS} protoc grpc_cpp_plugin
                COMMENT "Generating C++ code from ${PROTO_NAME}.proto"
                VERBATIM
        )
    endforeach()

    add_library(proto_lib STATIC
            ${PROTO_SRCS}
            ${PROTO_HDRS}
            ${GRPC_SRCS}
            ${GRPC_HDRS}
    )

    target_include_directories(proto_lib
            PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_link_libraries(proto_lib
            PUBLIC
            grpc++
            grpc++_reflection
            protobuf::libprotobuf
    )

    set_target_properties(proto_lib PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )

    add_dependencies(proto_lib protoc grpc_cpp_plugin)

else()
    message(WARNING "No .proto files found in ${CMAKE_CURRENT_SOURCE_DIR}")
    message(STATUS "Creating empty proto_lib interface library")

    add_library(proto_lib INTERFACE)
    target_link_libraries(proto_lib INTERFACE
            grpc++
            grpc++_reflection
            protobuf::libprotobuf
    )
endif()