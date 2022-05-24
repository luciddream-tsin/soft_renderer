cmake_minimum_required(VERSION 3.22)
project(soft_render)

add_library(tga STATIC ext/tgaimage.cpp ext/tgaimage.h)
target_include_directories(tga PUBLIC ext)

add_library(model STATIC ext/model.cpp ext/model.h ext/geometry.h)
target_include_directories(model PUBLIC ext)


add_subdirectory(lesson-0)