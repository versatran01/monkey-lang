cmake_minimum_required(VERSION 3.15)
project(monkey)

set(CMAKE_CXX_STANDARD 17)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(cmake/CompilerWarnings.cmake)
add_library(${PROJECT_NAME}_warnings INTERFACE)
set_project_warnings(${PROJECT_NAME}_warnings)

find_package(fmt REQUIRED)
find_package(glog REQUIRED)
find_package(absl REQUIRED)

add_library(monkey_lexer src/token.cpp src/lexer.cpp)
target_include_directories(monkey_lexer PUBLIC include)
target_link_libraries(
  monkey_lexer
  PUBLIC glog::glog fmt::fmt
  PRIVATE absl::flat_hash_map)

add_library(monkey_parser src/ast.cpp src/parser.cpp)
target_link_libraries(
  monkey_parser
  PUBLIC monkey_lexer absl::flat_hash_map
  PRIVATE absl::strings)

add_library(monkey_repl src/repl.cpp)
target_link_libraries(monkey_repl PUBLIC monkey_lexer)

enable_testing()
add_subdirectory(tests)
