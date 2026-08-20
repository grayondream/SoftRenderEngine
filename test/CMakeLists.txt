
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(LINKED_LIBRARIES gtest pthread math render core)

file(GLOB_RECURSE ALL_TEST_FILES "${SGE_TEST_DIR}/*.cpp")
if(ENABLE_SANITIZER)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

foreach(TEST_FILE ${ALL_TEST_FILES})
    get_filename_component(TARGET_NAME ${TEST_FILE} NAME_WE)
    get_filename_component(PATH_NAME ${TEST_FILE} DIRECTORY)
    get_filename_component(PATH_NAME ${PATH_NAME} NAME_WE)
    set(TARGET_NAME "${PATH_NAME}_${TARGET_NAME}")
    message(STATUS "Create Test Program ${TARGET_NAME} from ${TEST_FILE}")
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_compile_options(${TARGET_NAME} PRIVATE ${SGE_WARNING_FLAGS})
    target_link_libraries(${TARGET_NAME} ${LINKED_LIBRARIES})
endforeach()