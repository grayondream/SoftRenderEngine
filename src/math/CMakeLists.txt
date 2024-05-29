set(MATRIX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/math)
aux_source_directory(${MATRIX_SOURCE_DIR} MATRIX_SRC_FILES)
message(STATUS "libmatrix source files are: ${MATRIX_SRC_FILES}")
add_library(math ${MATRIX_SRC_FILES})