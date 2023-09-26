cmake_minimum_required(VERSION 3.22)
project(soft_render)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bins)

add_library(tga STATIC ext/tgaimage.cpp ext/tgaimage.h)
target_include_directories(tga PUBLIC ext)

add_library(model STATIC ext/model.cpp ext/model.h ext/geometry.h)
target_include_directories(model PUBLIC ext)


add_subdirectory(lesson-0)
add_subdirectory(lesson-1)
add_subdirectory(lesson-2)
add_subdirectory(lesson-3)
add_subdirectory(lesson-4-5)
add_subdirectory(lesson-6)
add_subdirectory(lesson-7)