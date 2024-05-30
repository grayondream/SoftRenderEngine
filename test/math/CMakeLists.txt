
add_executable(matrix_base MatrixBase.cpp)
target_link_libraries(matrix_base   gtest
                                    gtest_main
                                    gmock
                                    gmock_main
                                    pthread
                                    math)

get_target_property(LINKED_LIBS matrix_base LINK_LIBRARIES)
