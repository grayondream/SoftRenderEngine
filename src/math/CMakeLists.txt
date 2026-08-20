set(MATH_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/math)

set(MATRIX_SOURCE_DIR ${MATH_SOURCE_DIR}/Matrix)
set(VECTOR_SOURCE_DIR ${MATH_SOURCE_DIR}/Vector)
set(PRIMITIVE_SOURCE_DIR ${MATH_SOURCE_DIR}/Primitive)

aux_source_directory(${MATRIX_SOURCE_DIR} MATRIX_SRC_FILES)
aux_source_directory(${MATH_SOURCE_DIR} MATH_SRC_FILES)
aux_source_directory(${PRIMITIVE_SOURCE_DIR} PRIMITIVE_SRC_FILES)

message(STATUS "libmatrix source files are: ${MATRIX_SRC_FILES}")
add_library(math ${MATRIX_SRC_FILES} ${MATH_SRC_FILES} ${PRIMITIVE_SRC_FILES})
target_compile_options(math PRIVATE ${SGE_WARNING_FLAGS})
