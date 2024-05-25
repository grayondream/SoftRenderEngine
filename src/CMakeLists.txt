

set(APP_SRC_DIR ${CMAKE_SOURCE_DIR}/src)


add_subdirectory(matrix)

aux_source_directory(${APP_SRC_DIR} APP_SRC_FILES)

add_executable(${PROJECT_NAME}  ${APP_SRC_FILES})

target_link_libraries(${PROJECT_NAME}   SDL2
                                        matrix)