cmake_minimum_required(VERSION 3.15)
project(Projeto_GRPC)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

file(GLOB GURUX_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/gurux.dlms.cpp/development/src/*.cpp"
)

add_subdirectory(proto)

add_library(${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/generated
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gurux.dlms.cpp/development/include
)

target_sources(${PROJECT_NAME}_lib 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/XDlms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/service/frame_service.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server/server.cpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CommandTypes/ParseHeader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CommandTypes/GET/REQUEST/GetRequestParser.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CommandTypes/ACTIONS/REQUEST/ActionRequestParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/CommandTypes/ACTIONS/RESPONSE/ActionResponseParser.cpp

        ${GURUX_SOURCES}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME}_lib PRIVATE "/DGetUserNameA=GetUserName")
    target_compile_definitions(${PROJECT_NAME}_lib PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

target_link_libraries(${PROJECT_NAME}_lib 
    PUBLIC 
        gRPC::grpc++ 
        protobuf::libprotobuf
        proto-objects
)

add_executable(${PROJECT_NAME}_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server/main.cpp
)

target_link_libraries(${PROJECT_NAME}_server
    PRIVATE
        ${PROJECT_NAME}_lib
)

if(MSVC)
    set_target_properties(${PROJECT_NAME}_server PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

add_subdirectory(test)