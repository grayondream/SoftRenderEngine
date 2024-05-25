set(PROJECT_NAME soft-game-engine)
cmake_minimum_required(VERSION 3.20)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
include(src/include.cmake)

option(ENABLE_TEST "test basic ability" ON)

add_subdirectory(src)
add_subdirectory(test)
