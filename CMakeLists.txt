cmake_minimum_required(VERSION 3.18)

project(minijson)

set(SRC src/main.cc)

add_executable(${PROJECT_NAME} ${SRC})