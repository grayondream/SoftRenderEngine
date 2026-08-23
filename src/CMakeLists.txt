


set(SGE_MATH_DIR ${SGE_SOURCE_DIR}/math)
set(SGE_RENDER_DIR ${SGE_SOURCE_DIR}/Render)

set(SGE_GEOMATH_DIR ${SGE_SOURCE_DIR}/GeoObject)

add_subdirectory(${SGE_MATH_DIR})
add_subdirectory(${SGE_RENDER_DIR})

aux_source_directory(${SGE_SOURCE_DIR} APP_SRC_FILES)
list(REMOVE_ITEM APP_SRC_FILES ${SGE_SOURCE_DIR}/main.cpp)

file(GLOB SGE_SAMPLES_SRC ${SGE_SOURCE_DIR}/Samples/*.cpp)
list(APPEND APP_SRC_FILES ${SGE_SAMPLES_SRC})

add_library(core ${APP_SRC_FILES})
target_include_directories(core PRIVATE ${SGE_SOURCE_DIR}/Samples)
target_compile_options(core PRIVATE ${SGE_WARNING_FLAGS})
target_link_libraries(core PUBLIC SDL2 math render imgui)

add_executable(${PROJECT_NAME} ${SGE_SOURCE_DIR}/main.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE ${SGE_WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME} core)