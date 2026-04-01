cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(Ila)

add_executable(ila main.cpp)
target_compile_options(ila PRIVATE -fsanitize=address,undefined -ggdb)
target_link_options(ila PRIVATE -fsanitize=address,undefined)
