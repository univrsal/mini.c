cmake_minimum_required(VERSION 3.5)

option(ENABLE_DEMO "Enable demo program (default: ON)" ON)
project(minic VERSION 1.0 LANGUAGES C)

include_directories(src)
add_library("minic" STATIC src/mini.c src/mini.h)

if (ENABLE_DEMO)
add_executable(minitest
    example/demo.c)
    add_dependencies(minitest minic)
    target_link_libraries(minitest minic)
endif()

if(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
endif()