set(SGE_RENDER_GEOOBJECT_DIR ${SGE_RENDER_DIR}/GeoObject)
set(SGE_RENDER_SRC_FILES
    ${SGE_RENDER_DIR}/Rasterizer.cpp
    ${SGE_RENDER_DIR}/Pipeline.cpp
    ${SGE_RENDER_DIR}/Texture.cpp
    ${SGE_RENDER_DIR}/Light.cpp
    ${SGE_RENDER_DIR}/Camera.cpp
    ${SGE_RENDER_DIR}/ObjLoader.cpp
    ${SGE_RENDER_DIR}/RayTrace.cpp
)

aux_source_directory(${SGE_RENDER_GEOOBJECT_DIR} GEMO_SRC_FILES)


add_library(render ${GEMO_SRC_FILES} ${SGE_RENDER_SRC_FILES})
target_compile_options(render PRIVATE ${SGE_WARNING_FLAGS})
target_link_libraries(render PUBLIC uuid)
