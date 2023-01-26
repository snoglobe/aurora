cmake_minimum_required(VERSION 3.22)
project(aurora)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
set(CMAKE_CXX_FLAGS_DEBUG  "-g")

add_executable(aurora main.cpp context.cpp lexer.cpp aurora_obj.h)
