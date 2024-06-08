


set(SGE_MATH_DIR ${SGE_SOURCE_DIR}/math)

add_subdirectory(${SGE_MATH_DIR})

aux_source_directory(${SGE_SOURCE_DIR} APP_SRC_FILES)
add_executable(${PROJECT_NAME}  ${APP_SRC_FILES})
target_link_libraries(${PROJECT_NAME}   SDL2 math)