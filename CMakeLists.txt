cmake_minimum_required(VERSION 3.25)
project(aac C)

set(CMAKE_C_STANDARD 11)

add_executable(aac main.c
        utils.h
        utils.c
        graph.h
        graph.c)

