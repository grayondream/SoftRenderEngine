set(MATRIX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/matrix)
aux_source_directory(${MATRIX_SOURCE_DIR} MATRIX_SRC_FILES)

add_library(matrix ${MATRIX_SRC_FILES})