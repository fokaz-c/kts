cmake_minimum_required(VERSION 3.24)

project(gameboy_emulator LANGUAGES CXX)

include(FetchContent)
include(CTest)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(gameboy_core INTERFACE)
add_library(gameboy::core ALIAS gameboy_core)
target_compile_features(gameboy_core INTERFACE cxx_std_23)
target_include_directories(gameboy_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        FIND_PACKAGE_ARGS NAMES GTest
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(gameboy_core_tests
        src/gameboy/cpu/registers.cpp
        tests/unit/test_cycle_count.cpp
        tests/unit/test_registers.cpp
    )
    target_link_libraries(gameboy_core_tests
        PRIVATE
            gameboy::core
            GTest::gtest_main
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(gameboy_core_tests
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
        )
    elseif(MSVC)
        target_compile_options(gameboy_core_tests
            PRIVATE
                /W4
                /permissive-
        )
    endif()

    include(GoogleTest)
    gtest_discover_tests(gameboy_core_tests)
endif()
