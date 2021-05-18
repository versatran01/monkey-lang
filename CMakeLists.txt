cmake_minimum_required(VERSION 3.15)
project(monkey)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerWarnings)
add_library(monkey_warnings INTERFACE)
set_project_warnings(monkey_warnings)

find_package(fmt REQUIRED)
find_package(glog REQUIRED)
find_package(absl REQUIRED)

set(CC_TARGET_PREFIX ${PROJECT_NAME})
include(BazelHelpers)
include(Cache)

option(BUILD_SHARED_LIBS "Build shared libraries" On)
option(BUILD_TESTING "Build testing" On)

include_directories(include)
add_subdirectory(src)
add_subdirectory(cli)

enable_testing()
add_subdirectory(tests)
