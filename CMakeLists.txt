cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()
if(NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER clang)
endif()

set(PROJECT_NAME soft-game-engine)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# default to an optimized build: a plain `cmake -B build` used to produce
# -O0 binaries, which made the software rasterizer unusably slow
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)
endif()

# CPU acceleration: let the compiler emit AVX2/FMA for the hot math and
# rasterizer loops (auto-vectorization). Turn off for portable binaries.
option(SGE_NATIVE_ARCH "compile with -march=native" ON)
if(SGE_NATIVE_ARCH AND CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    add_compile_options(-march=native)
endif()

set(SGE_WARNING_FLAGS -Wall -Wextra -Werror CACHE STRING "warning flags for project targets")

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo|MinSizeRel")
    message(STATUS "NDEBUG build detected: keeping asserts enabled project-wide (defensive checks and death tests rely on them)")
    add_compile_options(-UNDEBUG)
endif()

set(SGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src CACHE INTERNAL "Source Directory")
set(SGE_TEST_DIR ${CMAKE_SOURCE_DIR}/test CACHE INTERNAL "Test Source Directory")

include(${SGE_SOURCE_DIR}/include.cmake)

option(ENABLE_TEST "test basic ability" ON)
option(ENABLE_SANITIZER "build everything with ASan+UBSan" OFF)

if(ENABLE_SANITIZER)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party)
add_subdirectory(${SGE_SOURCE_DIR})
if(ENABLE_TEST)
    enable_testing()
    add_subdirectory(${SGE_TEST_DIR})
    add_subdirectory(${CMAKE_SOURCE_DIR}/tools)
endif()
