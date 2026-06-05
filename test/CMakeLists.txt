cmake_minimum_required(VERSION 3.16)

project(tests)

add_executable(${PROJECT_NAME})

find_package(Catch2 CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Catch2::Catch2WithMain
        Projeto_GRPC_lib            
)

target_sources(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_ActionsRequestVerifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_ActionsResponseVerifier.cpp

)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Projeto_GRPC_lib
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/generated
        ${CMAKE_SOURCE_DIR}/test/include
)