cmake_minimum_required(VERSION 3.5)
project(avlexample)

find_package(
        AVL
        # for a specific version, uncomment the line below
        #4.11
        CONFIG
        REQUIRED
)

add_executable(
        # executable name
        example_exec
        # source files
        main.cpp
)

target_link_libraries(
        example_exec
        PUBLIC
        AVL
)