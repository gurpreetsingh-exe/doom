cmake_minimum_required(VERSION 3.7)

project(doom VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_C_FLAGS "-Wall -Werror")

set(SOURCES
    lib/main.c
    lib/window.c
    lib/image.c
    lib/map.c
    lib/renderer.c
    lib/map_renderer.c
    lib/wad.c
    lib/player.c
    lib/engine.c
    lib/utils.c)

add_executable(doom ${SOURCES})
include_directories(include)

add_subdirectory(extern/glfw)
add_subdirectory(extern/glew/build/cmake)

target_link_libraries(doom PUBLIC glfw glew)

target_compile_options(doom PRIVATE -ggdb -fsanitize=address -fno-omit-frame-pointer)
target_link_options(doom PRIVATE -fsanitize=address)
