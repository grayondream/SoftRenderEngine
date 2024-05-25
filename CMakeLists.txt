set(PROJECT_NAME soft-game-engine)
cmake_minimum_required(VERSION 3.20)

project(${PROJECT_NAME})

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0 -std=c++11")

include(src/include.cmake)

option(ENABLE_TEST "test basic ability" ON)

add_subdirectory(src)
add_subdirectory(test)
