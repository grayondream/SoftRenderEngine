cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

set(PROJECT_NAME soft-game-engine)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(SGE_WARNING_FLAGS -Wall -Wextra -Werror CACHE STRING "warning flags for project targets")

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo|MinSizeRel")
    message(STATUS "NDEBUG build detected: keeping asserts enabled project-wide (defensive checks and death tests rely on them)")
    add_compile_options(-UNDEBUG)
endif()

set(SGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src CACHE INTERNAL "Source Directory")
set(SGE_TEST_DIR ${CMAKE_SOURCE_DIR}/test CACHE INTERNAL "Test Source Directory")

include(${SGE_SOURCE_DIR}/include.cmake)

option(ENABLE_TEST "test basic ability" ON)

add_subdirectory(${SGE_SOURCE_DIR})
if(ENABLE_TEST)
    add_subdirectory(${SGE_TEST_DIR})
endif()
