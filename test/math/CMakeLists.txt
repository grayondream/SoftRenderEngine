
set(LINKED_LIBRARIES gtest gtest_main gmock gmock_main pthread math)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(matrix_base MatrixBase.cpp)
target_link_libraries(matrix_base ${LINKED_LIBRARIES})

add_executable(matrix_op MatrixMathOp.cpp)
target_link_libraries(matrix_op ${LINKED_LIBRARIES})

add_executable(vector_op VectorOp.cpp)
target_link_libraries(vector_op ${LINKED_LIBRARIES})

add_executable(static_matrix_op StaticMatrix.cpp)
target_link_libraries(static_matrix_op ${LINKED_LIBRARIES})

get_target_property(LIBA_INCLUDES static_matrix_op INCLUDE_DIRECTORIES)
message(STATUS ${LIBA_INCLUDES})
