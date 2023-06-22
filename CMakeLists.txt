cmake_minimum_required(VERSION 3.24)
set(CMAKE_C_VISIBILITY_PRESET hidden)
project(hak)

set(CMAKE_CXX_STANDARD 23)

# PROJECT SETTING
set(BUILD_USE_64BITS ON)
set(BUILD_UNIT_TEST ON)
set(BUILD_JNI OFF)

# PLATFORM SETTING
set(HAKUTAKU_64BIT ON) # 64bit platform
set(IGNORE_MISSING_PAGE OFF) # ignore missing page
set(SUPPORT_UNALIGNED_MEMORY OFF) # support unaligned memory access

if(HAKUTAKU_64BIT)
    message("-- Compile for 64bit platform")
    add_definitions(-DHAKUTAKU_64BIT)
endif()

if(SUPPORT_UNALIGNED_MEMORY)
    message("-- Compile for supporting unaligned memory access")
    add_definitions(-DSUPPORT_UNALIGNED_MEMORY)
endif()

if(IGNORE_MISSING_PAGE)
    message("-- Compile for ignoring missing page")
    add_definitions(-DIGNORE_MISSING_PAGE)
endif()

add_subdirectory(core)
add_subdirectory(jni)

if(BUILD_UNIT_TEST)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.13.0
    )
    FetchContent_MakeAvailable(googletest)

    #add_subdirectory(library/googletest)
    #include_directories(googletest/include googletest)

    message("-- Build unit test")

    add_executable(${PROJECT_NAME} test/test.cpp)

    target_link_libraries(${PROJECT_NAME} hak::core)
    target_link_libraries(${PROJECT_NAME} gtest gtest_main)
elseif (BUILD_JNI)
    message("-- Build for jni")

    add_library(${PROJECT_NAME} SHARED
            jni/jni_proc.cpp
    )

    target_link_libraries(${PROJECT_NAME} hak::core)
    target_link_libraries(${PROJECT_NAME} hak::jni)
else()
    message("-- Build common")
    add_executable(${PROJECT_NAME} main.cpp)
endif()

