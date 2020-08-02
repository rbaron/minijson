cmake_minimum_required(VERSION 3.18)

project(minijson)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(SRC src/main.cc
        src/lexer.cc
)

add_executable(${PROJECT_NAME} ${SRC})