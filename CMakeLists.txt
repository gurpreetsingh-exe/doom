cmake_minimum_required(VERSION 3.7)

project(doom VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_C_FLAGS "-Wall -Werror")

set(IMGUI_DIR extern/cimgui/imgui)

set(SOURCES
    lib/main.c
    lib/window.c
    lib/image.c
    lib/map.c
    lib/renderer.c
    lib/map_renderer.c
    lib/view_renderer.c
    lib/wad.c
    lib/player.c
    lib/engine.c
    lib/utils.c
    lib/imgui_layer.c
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)

add_executable(doom ${SOURCES})
include_directories(include)
include_directories(extern/cimgui/generator/output/)

add_subdirectory(extern/glfw)
add_subdirectory(extern/glew/build/cmake)
add_subdirectory(extern/cimgui)

target_link_libraries(doom PUBLIC glfw glew cimgui)

target_compile_options(doom PRIVATE -ggdb -fsanitize=address -fno-omit-frame-pointer)
target_link_options(doom PRIVATE -fsanitize=address)
