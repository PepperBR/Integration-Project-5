cmake_minimum_required(VERSION 3.15)
project(Projeto_GRPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_subdirectory(proto)

add_library(${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

target_sources(${PROJECT_NAME}_lib 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/verifier.cpp
)


target_link_libraries(${PROJECT_NAME}_lib 
    PUBLIC 
        gRPC::grpc++ 
        protobuf::libprotobuf
        proto-objects
)

add_subdirectory(test)