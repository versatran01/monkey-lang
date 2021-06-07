cmake_minimum_required(VERSION 3.15)
project(monkey)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(BUILD_SHARED_LIBS "Build shared libraries" On)
option(BUILD_TESTING "Build testing" On)

set(CC_TARGET_PREFIX ${PROJECT_NAME})
include(CMakeHelpers)
add_library(monkey_options INTERFACE)
enable_warnings(monkey_options)
enable_sanitizers(monkey_options)

# dep
find_package(fmt REQUIRED)
find_package(glog REQUIRED)
find_package(absl REQUIRED)

# test
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
find_package(benchmark REQUIRED)

include_directories(include)
add_subdirectory(src)
add_subdirectory(cli)

enable_testing()
add_subdirectory(tests)
