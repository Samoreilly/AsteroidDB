cmake_minimum_required(VERSION 4.1)
project(AsteroidDB)

set(CMAKE_CXX_STANDARD 20)

add_executable(
  AsteroidDB core/main.cpp          
  core/sql/lexer/lexer.cpp)

