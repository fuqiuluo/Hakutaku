cmake_minimum_required(VERSION 3.24)
set(CMAKE_C_VISIBILITY_PRESET hidden)
project(Hakutaku)

set(CMAKE_CXX_STANDARD 23)

# PROJECT SETTING
set(BUILD_UNIT_TEST ON)

# PLATFORM SETTING
set(HAKUTAKU_64BIT ON)

if(HAKUTAKU_64BIT)
    message("-- Compile for 64bit platform")
    add_definitions(-DHAKUTAKU_64BIT)
endif()

if(BUILD_UNIT_TEST)
    add_subdirectory(library/googletest)
    include_directories(googletest/include googletest)

    message("-- Build unit test")
    add_executable(Hakutaku test/test.cpp
            core/Hakutaku.hpp)

    target_link_libraries(Hakutaku gtest gtest_main)
else()
    message("-- Build common")
    add_executable(Hakutaku main.cpp
            core/Hakutaku.hpp)
endif()

