
add_executable(golden_render golden_render.cpp)
target_compile_options(golden_render PRIVATE ${SGE_WARNING_FLAGS})
target_link_libraries(golden_render render math core)

add_test(NAME golden_image
         COMMAND ${CMAKE_SOURCE_DIR}/scripts/check_golden.sh $<TARGET_FILE:golden_render> ${CMAKE_SOURCE_DIR}/assets/golden/lit_cube.md5)
