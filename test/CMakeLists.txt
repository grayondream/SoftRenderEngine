
set(LINKED_LIBRARIES gtest gtest_main gmock gmock_main pthread math)

file(GLOB_RECURSE ALL_TEST_FILES "${SGE_TEST_DIR}/*.cpp")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

foreach(TEST_FILE ${ALL_TEST_FILES})
    get_filename_component(TARGET_NAME ${TEST_FILE} NAME_WE)
    get_filename_component(PATH_NAME ${TEST_FILE} DIRECTORY)
    get_filename_component(PATH_NAME ${PATH_NAME} NAME_WE)
    set(TARGET_NAME "${PATH_NAME}_${TARGET_NAME}")
    message(STATUS "Create Test Program ${TARGET_NAME} from ${TEST_FILE}")
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TARGET_NAME} ${LINKED_LIBRARIES})
endforeach()