cmake_minimum_required(VERSION 3.25)
project(gash)

set(CMAKE_CXX_STANDARD 20)

add_executable(gash main.cpp lexer.cpp lexer.h token.cpp token.h)
