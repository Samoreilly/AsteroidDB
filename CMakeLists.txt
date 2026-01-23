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
  core/sql/parser/Insert.cpp
  core/engine/storage/Page.cpp
  core/engine/storage/Record.cpp
  core/engine/storage/PageManager.cpp
  core/engine/storage/BufferPool.cpp
  core/engine/storage/TableHeap.cpp
  core/engine/executor/Catalog.cpp
  core/engine/executor/ExecutorEngine.cpp
  core/engine/executor/CreateExecutor.cpp
  core/engine/executor/InsertExecutor.cpp
  core/engine/executor/SelectExecutor.cpp
  core/engine/executor/DeleteExecutor.cpp)

