cmake_minimum_required(VERSION 4.1)
project(AsteroidDB)

set(CMAKE_CXX_STANDARD 20)

add_executable(
  AsteroidDB core/main.cpp          
  core/sql/lexer/lexer.cpp
  core/sql/ast/Parser.cpp
  core/sql/ast/Node.cpp
  core/sql/parser/Select.cpp
  core/sql/parser/Create.cpp
  core/sql/parser/Insert.cpp)

