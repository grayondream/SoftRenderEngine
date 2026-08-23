
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)
target_include_directories(imgui SYSTEM PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
execute_process(COMMAND sdl2-config --cflags OUTPUT_VARIABLE SGE_SDL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(SGE_SDL_CFLAGS)
target_compile_options(imgui PRIVATE ${SGE_SDL_CFLAGS})
target_compile_options(imgui PRIVATE -w)
target_link_libraries(imgui PUBLIC SDL2)
