cmake_minimum_required(VERSION 3.23)
project (hak_core)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} ${SOURCES})
add_library(hak::core ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include)