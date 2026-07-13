cmake_minimum_required(VERSION 4.2)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

# Enable experimental C++ modules support for GCC/Clang where available
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fmodules -fmodules-ts)
endif()

add_executable(untitled
        Utils.cppm
        Turtle.cppm
        TurtleRegistry.cppm
        ResponseMessages.cppm
        WorldMap.cppm
        MessageHandlers.cppm
        Globals.cppm
        main.cpp)
