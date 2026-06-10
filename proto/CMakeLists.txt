cmake_minimum_required(VERSION 3.15)
project(GenProto)

# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_library(proto-objects 
    OBJECT 
        "${CMAKE_CURRENT_LIST_DIR}/frame.proto"
)

target_link_libraries(proto-objects 
    PUBLIC 
        protobuf::libprotobuf 
        gRPC::grpc++
)

set(PROTO_BINARY_DIR ${CMAKE_SOURCE_DIR}/generated)
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}")

target_include_directories(proto-objects PUBLIC $<BUILD_INTERFACE:${PROTO_BINARY_DIR}>)

protobuf_generate(
    TARGET proto-objects
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

protobuf_generate(
    TARGET proto-objects
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

