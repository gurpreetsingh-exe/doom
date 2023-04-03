cmake_minimum_required(VERSION 3.7)

project(doom VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)

set(SOURCES lib/main.c lib/window.c)

add_executable(doom ${SOURCES})
include_directories(include/)

add_subdirectory(extern/glfw)

target_link_libraries(doom PUBLIC glfw)
# target_compile_options(doom PRIVATE -ggdb -fsanitize=address -fno-omit-frame-pointer -pthread)
# target_link_options(doom PRIVATE -fsanitize=address)

target_compile_options(doom PRIVATE -ggdb -fsanitize=address -fno-omit-frame-pointer)
target_link_options(doom PRIVATE -fsanitize=address)
