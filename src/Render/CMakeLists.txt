set(SGE_RENDER_GEOOBJECT_DIR ${SGE_RENDER_DIR}/GeoObject)

aux_source_directory(${SGE_RENDER_GEOOBJECT_DIR} GEMO_SRC_FILES)


add_library(render ${GEMO_SRC_FILES})