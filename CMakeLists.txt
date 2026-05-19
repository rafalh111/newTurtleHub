cmake_minimum_required(VERSION 4.2)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled main.cpp
        WorldMap.cpp
        WorldMap.h
        Turtle.cpp
        Turtle.h
        Utils.h
        MessageHandlers.cpp
        MessageHandlers.h
        TurtleRegistry.cpp
        TurtleRegistry.h
        Globals.cpp
        Globals.h
        Utils.cpp
        ResponseMessages.h)
