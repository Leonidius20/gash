cmake_minimum_required(VERSION 3.25)
project(gash)

set(CMAKE_CXX_STANDARD 20)

add_executable(gash main.cpp lexer/lexer.cpp lexer/lexer.h lexer/token.cpp lexer/token.h ast/tree_node.cpp ast/tree_node.h ast/visitor.cpp ast/visitor.h interpreter/interpreter.h interpreter/run_command.cpp interpreter/run_pipeline.cpp interpreter/execute_program.cpp)
