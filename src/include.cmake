
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
include_directories(${SOURCE_DIR})

set(MATH_DIR ${SOURCE_DIR}/math)
set(RENDER_DIR ${SOURCE_DIR}/Render)

include_directories(${MATH_DIR})
include_directories(${MATH_DIR}/Matrix)
include_directories(${MATH_DIR}/Primitive)
include_directories(${MATH_DIR}/Util)
include_directories(${MATH_DIR}/Vector)
include_directories(${MATH_DIR}/Geometry)
include_directories(${MATH_DIR}/Math)
include_directories(${RENDER_DIR})
include_directories(${RENDER_DIR}/GeoObject)

include_directories(${CMAKE_SOURCE_DIR}/third_party)
include_directories(${CMAKE_SOURCE_DIR}/third_party/stb)



