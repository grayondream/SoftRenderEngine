
message(STATUS "Generate executable programe ${PROJECT_NAME}")

if (UNIX)
    set(PLATFORM_DIR Linux)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/${PLATFORM_DIR})
message(STATUS "include directory ${PROJECT_SOURCE_DIR}/src/${PLATFORM_DIR}")

add_executable(${PROJECT_NAME}  main.cpp
                                Window.cpp
                                Application.cpp
                                Environment.cpp
                                WindowBuffer.cpp
                                BufferManager.cpp)

target_link_libraries(${PROJECT_NAME} SDL2)