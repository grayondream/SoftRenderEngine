
add_executable(matrix_base MatrixBase.cpp)
target_link_libraries(matrix_base   gtest
                                    gtest_main
                                    gmock
                                    gmock_main
                                    math)

