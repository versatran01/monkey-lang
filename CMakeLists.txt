cmake_minimum_required(VERSION 3.15)
project(monkey)

find_package(absl REQUIRED)
find_package(glog REQUIRED)
find_package(fmt REQUIRED)

add_library(monkey src/lexer.cpp src/token.cpp)
target_include_directories(monkey PUBLIC include)

enable_testing()
add_subdirectory(tests)
