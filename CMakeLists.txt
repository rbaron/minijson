cmake_minimum_required(VERSION 3.18)

project(minijson)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_subdirectory(tests)
add_subdirectory(examples)