cmake_minimum_required(VERSION 3.5)

project(minic VERSION 1.0 LANGUAGES C)

include_directories(src)

add_executable(minitest src/mini.c
    src/mini.h
    example/demo.c)

