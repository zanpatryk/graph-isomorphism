cmake_minimum_required(VERSION 3.25)
project(aac C)

set(CMAKE_C_STANDARD 11)

add_executable(aac main.c
        utils.h
        utils.c
        graph.h
        graph.c
        algorithms.h
        algorithms/minimal_extension.h
        algorithms/minimal_extension.c
        algorithms/minimal_extension_approximation.c
        algorithms/minimal_extension_approximation.h
        algorithms/product_graph.h
        algorithms/product_graph.c
        algorithms/isomorphism.h
        algorithms/isomorphism.c
        test/graph_gen.c)

