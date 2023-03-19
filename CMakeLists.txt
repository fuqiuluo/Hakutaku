cmake_minimum_required(VERSION 3.24)
project(Hakutaku)

set(CMAKE_CXX_STANDARD 23)

set(HAKUTAKU_64BIT ON)
#set(HAKUTAKU_BIT64 OFF)


if(HAKUTAKU_64BIT)
    message("-- Compile for 64bit platform")
    add_definitions(-DHAKUTAKU_64BIT)
endif()

add_executable(Hakutaku main.cpp
        core/Hakutaku.hpp)
