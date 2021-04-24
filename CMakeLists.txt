cmake_minimum_required(VERSION 3.15)
project(monkey)

set(CMAKE_CXX_STANDARD 17)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(cmake/CompilerWarnings.cmake)
add_library(${PROJECT_NAME}_warnings INTERFACE)
set_project_warnings(${PROJECT_NAME}_warnings)

find_package(absl REQUIRED)
find_package(glog REQUIRED)
find_package(fmt REQUIRED)

add_library(monkey src/lexer.cpp src/token.cpp src/repl.cpp src/ast.cpp
                   src/parser.cpp)
target_include_directories(monkey PUBLIC include)
target_link_libraries(monkey PUBLIC absl::flat_hash_map fmt::fmt glog::glog)

add_executable(monkey_repl src/repl_main.cpp)
target_link_libraries(monkey_repl PUBLIC monkey)

enable_testing()
add_subdirectory(tests)
