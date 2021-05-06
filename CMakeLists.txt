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

add_subdirectory(src)
add_subdirectory(example)

enable_testing()
add_subdirectory(tests)
