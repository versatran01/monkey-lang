# cmake-format: off
# cc_library()
#
# CMake function to imitate Bazel's cc_library rule.
#
# Parameters:
# NAME: name of target (see Note)
# HDRS: List of public header files for the library
# SRCS: List of source files for the library
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# 
# By default, cc_library will always create a library named 
# ${CC_TARGET_PREFIX}_${NAME}, and alias target ${CC_TARGET_PREFIX}::${NAME}.  
# The ${CC_TARGET_PREFIX}:: form should always be used.
# This is to reduce namespace pollution.
#
# cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
# )
# cc_library(
#   NAME
#     fantastic_lib
#   SRCS
#     "b.cc"
#   DEPS
#     sv::awesome # not "awesome" !
# )
#
# cc_library(
#   NAME
#     main_lib
#   ...
#   DEPS
#     sv::fantastic_lib
# )
# cmake-format: on
function(cc_library)
  cmake_parse_arguments(CC_LIB "INTERFACE" "NAME"
                        "HDRS;SRCS;COPTS;DEFINES;LINKOPTS;DEPS;INCDIRS" ${ARGN})

  if(CC_TARGET_PREFIX)
    set(_NAME "${CC_TARGET_PREFIX}_${CC_LIB_NAME}")
  else()
    set(_NAME ${CC_LIB_NAME})
  endif()

  # Check if this is a header-only library Note that as of February 2019, many
  # popular OS's (for example, Ubuntu 16.04 LTS) only come with cmake 3.5 by
  # default.  For this reason, we can't use list(FILTER...)
  set(SV_SRCS "${CC_LIB_SRCS}")
  foreach(src_file IN LISTS SV_SRCS)
    if(${src_file} MATCHES ".*\\.(h|hpp|inc)")
      list(REMOVE_ITEM SV_SRCS "${src_file}")
    endif()
  endforeach()

  if(CC_LIB_INTERFACE)
    set(CC_LIB_IS_INTERFACE 1)
  else()
    set(CC_LIB_IS_INTERFACE 0)
  endif()

  if(NOT CC_LIB_IS_INTERFACE)
    add_library(${_NAME} "")
    target_sources(${_NAME} PRIVATE ${CC_LIB_SRCS} ${CC_LIB_HDRS})
    target_link_libraries(
      ${_NAME}
      PUBLIC ${CC_LIB_DEPS}
      PRIVATE ${CC_LIB_LINKOPTS})

    # Linker language can be inferred from sources, but in the case of DLLs we
    # don't have any .cc files so it would be ambiguous. We could set it
    # explicitly only in the case of DLLs but, because "CXX" is always the
    # correct linker language for static or for shared libraries, we set it
    # unconditionally.
    set_property(TARGET ${_NAME} PROPERTY LINKER_LANGUAGE "CXX")

    target_include_directories(${_NAME} PUBLIC ${CC_LIB_INCDIRS})
    target_compile_options(${_NAME} PRIVATE ${CC_LIB_COPTS})
    target_compile_definitions(${_NAME} PUBLIC ${CC_LIB_DEFINES})

    # INTERFACE libraries can't have the CXX_STANDARD property set
    set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  else()
    # Generating header-only library
    add_library(${_NAME} INTERFACE)
    target_include_directories(${_NAME} INTERFACE ${CC_LIB_INCDIRS})

    target_link_libraries(${_NAME} INTERFACE ${CC_LIB_DEPS} ${CC_LIB_LINKOPTS})
    target_compile_definitions(${_NAME} INTERFACE ${CC_LIB_DEFINES})
  endif()

  if(CC_TARGET_PREFIX)
    add_library(${CC_TARGET_PREFIX}::${CC_LIB_NAME} ALIAS ${_NAME})
  endif()
endfunction()

# cmake-format: off
# cc_binary()
# adapted from absl_cc_test()
#
# Parameters:
# NAME: name of target (see Usage below)
# SRCS: List of source files for the binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# By default, cc_binary will always create a binary named 
# ${CC_TARGET_PREFIX}_${NAME}.
#
# Usage:
# cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
#   PUBLIC
# )
#
# cc_binary(
#   NAME
#     awesome_test
#   SRCS
#     "awesome_test.cc"
#   DEPS
#     sv::awesome
#     gmock
#     gtest_main
# )
# cmake-format: on
function(cc_binary)
  cmake_parse_arguments(CC_BIN "" "NAME" "SRCS;COPTS;DEFINES;LINKOPTS;DEPS"
                        ${ARGN})

  if(CC_TARGET_PREFIX)
    set(_NAME "${CC_TARGET_PREFIX}_${CC_BIN_NAME}")
  else()
    set(_NAME ${CC_BIN_NAME})
  endif()

  add_executable(${_NAME} "")
  target_sources(${_NAME} PRIVATE ${CC_BIN_SRCS})

  target_compile_definitions(${_NAME} PUBLIC ${CC_BIN_DEFINES})
  target_compile_options(${_NAME} PRIVATE ${CC_BIN_COPTS})

  target_link_libraries(
    ${_NAME}
    PUBLIC ${CC_BIN_DEPS}
    PRIVATE ${CC_BIN_LINKOPTS})

  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()

# cmake-format: off
# cc_test()
# adapted from absl_cc_test()
#
# Parameters:
# NAME: name of target (see Usage below)
# SRCS: List of source files for the binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# By default, cc_test will always create a binary named ${CC_TARGET_PREFIX}_${NAME}.
# This will also add it to ctest list as ${CC_TARGET_PREFIX}_${NAME}.
#
# Usage:
# cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
#   PUBLIC
# )
#
# cc_test(
#   NAME
#     awesome_test
#   SRCS
#     "awesome_test.cc"
#   DEPS
#     sv::awesome
#     gmock
# )
# cmake-format: on
function(cc_test)
  cmake_parse_arguments(CC_TEST "CATKIN;OTHER" "NAME"
                        "SRCS;COPTS;DEFINES;LINKOPTS;DEPS" ${ARGN})

  if(CC_TARGET_PREFIX)
    set(_NAME "${CC_TARGET_PREFIX}_${CC_TEST_NAME}")
  else()
    set(_NAME ${CC_TEST_NAME})
  endif()

  if(CC_TEST_CATKIN)
    if(CATKIN_ENABLE_TESTING)
      catkin_add_gtest(${_NAME} ${CC_TEST_SRCS})
      target_link_libraries(${_NAME} ${CC_TEST_DEPS} ${CC_TEST_LINKOPTS}
                            GTest::Main)
    endif()
  else()
    if(NOT BUILD_TESTING)
      return()
    endif()

    add_executable(${_NAME} "")
    target_sources(${_NAME} PRIVATE ${CC_TEST_SRCS})

    target_compile_definitions(${_NAME} PUBLIC ${CC_TEST_DEFINES})
    target_compile_options(${_NAME} PRIVATE ${CC_TEST_COPTS})
    target_link_libraries(
      ${_NAME}
      PUBLIC ${CC_TEST_DEPS}
      PRIVATE ${CC_TEST_LINKOPTS})

    if(NOT CC_TEST_OTHER)
      target_link_libraries(${_NAME} PRIVATE GTest::GTest GTest::Main)
    endif()

    set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

    add_test(NAME ${_NAME} COMMAND ${_NAME})
    set_tests_properties(${_NAME} PROPERTIES FAIL_REGULAR_EXPRESSION
                                             ".*FAILED.*")
  endif()

endfunction()

# cmake-format: off
# cc_bench()
# adapted from absl_cc_test()
#
# Parameters:
# NAME: name of target (see Usage below)
# SRCS: List of source files for the binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# By default, cc_bench will always create a binary named ${CC_TARGET_PREFIX}_${NAME}.
# This will also add it to ctest list as ${CC_TARGET_PREFIX}_${NAME}.
#
# Usage:
# cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
#   PUBLIC
# )
#
# cc_bench(
#   NAME
#     awesome_bench
#   SRCS
#     "awesome_bench.cc"
#   DEPS
#     sv::awesome
# )
# cmake-format: on
function(cc_bench)
  if(NOT BUILD_TESTING)
    return()
  endif()

  cmake_parse_arguments(CC_BENCH "" "NAME" "SRCS;COPTS;DEFINES;LINKOPTS;DEPS"
                        ${ARGN})

  if(CC_TARGET_PREFIX)
    set(_NAME "${CC_TARGET_PREFIX}_${CC_BENCH_NAME}")
  else()
    set(_NAME ${CC_BENCH_NAME})
  endif()

  add_executable(${_NAME} "")
  target_sources(${_NAME} PRIVATE ${CC_BENCH_SRCS})

  target_compile_definitions(${_NAME} PUBLIC ${CC_BENCH_DEFINES})
  target_compile_options(${_NAME} PRIVATE ${CC_BENCH_COPTS})

  target_link_libraries(
    ${_NAME}
    PUBLIC ${CC_BENCH_DEPS}
    PRIVATE ${CC_BENCH_LINKOPTS} benchmark::benchmark benchmark::benchmark_main)

  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()


# static analyzers
option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(ENABLE_INCLUDE_WHAT_YOU_USE
       "Enable static analysis with include-what-you-use" OFF)

if(ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    set(CMAKE_CXX_CPPCHECK
        ${CPPCHECK}
        --suppress=missingInclude
        --enable=all
        --inline-suppr
        --inconclusive
        -i
        ${CMAKE_SOURCE_DIR}/include)
  else()
    message(SEND_ERROR "cppcheck requested but executable not found")
  endif()
endif()

if(ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY}
                             -extra-arg=-Wno-unknown-warning-option)
  else()
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  else()
    message(
      SEND_ERROR "include-what-you-use requested but executable not found")
  endif()
endif()

# Sanitizers
function(enable_sanitizers TARGET)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                             ".*Clang")
    option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

    if(ENABLE_COVERAGE)
      target_compile_options(${TARGET} INTERFACE --coverage -O0 -g)
      target_link_libraries(${TARGET} INTERFACE --coverage)
    endif()

    set(SANITIZERS "")

    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" FALSE)
    if(ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
    endif()

    option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" FALSE)
    if(ENABLE_SANITIZER_LEAK)
      list(APPEND SANITIZERS "leak")
    endif()

    option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR
           "Enable undefined behavior sanitizer" FALSE)
    if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
    endif()

    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" FALSE)
    if(ENABLE_SANITIZER_THREAD)
      if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
        message(
          WARNING
            "Thread sanitizer does not work with Address and Leak sanitizer enabled"
        )
      else()
        list(APPEND SANITIZERS "thread")
      endif()
    endif()

    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" FALSE)
    if(ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      if("address" IN_LIST SANITIZERS
         OR "thread" IN_LIST SANITIZERS
         OR "leak" IN_LIST SANITIZERS)
        message(
          WARNING
            "Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled"
        )
      else()
        list(APPEND SANITIZERS "memory")
      endif()
    endif()

    list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)

  endif()

  if(LIST_OF_SANITIZERS)
    if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
      target_compile_options(${TARGET}
                             INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
      target_link_options(${TARGET} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
    endif()
  endif()

endfunction()

# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
function(enable_warnings TARGET)

  set(CLANG_WARNINGS
      -Wall
      -Wextra # reasonable and standard
      # -Wshadow # warn the user if a variable declaration shadows one from a
      # parent context
      -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                         # non-virtual destructor. This helps catch hard to
                         # track down memory errors
      -Wold-style-cast # warn for c-style casts
      -Wcast-align # warn for potential performance problem casts
      -Wunused # warn on anything being unused
      -Woverloaded-virtual # warn if you overload (not override) a virtual
                           # function
      -Wpedantic # warn if non-standard C++ is used
      -Wconversion # warn on type conversions that may lose data
      -Wsign-conversion # warn on sign conversions
      -Wnull-dereference # warn if a null dereference is detected
      -Wdouble-promotion # warn if float is implicit promoted to double
      -Wformat=2 # warn on security issues around functions that format output
                 # (ie printf)
  )

  set(GCC_WARNINGS
      ${CLANG_WARNINGS}
      -Wmisleading-indentation # warn if indentation implies blocks where blocks
                               # do not exist
      -Wduplicated-cond # warn if if / else chain has duplicated conditions
      -Wduplicated-branches # warn if if / else branches have duplicated code
      -Wlogical-op # warn about logical operations being used where bitwise were
                   # probably wanted
      -Wuseless-cast # warn if you perform a cast to the same type
      -Wpessimizing-move
      -Wredundant-move)

  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  else()
    message(
      AUTHOR_WARNING
        "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
  endif()

  target_compile_options(${TARGET} INTERFACE ${PROJECT_WARNINGS})

endfunction()

option(ENABLE_CACHE "Enable cache if available" ON)
if(NOT ENABLE_CACHE)
  return()
endif()

set(CACHE_OPTION
    "ccache"
    CACHE STRING "Compiler cache to be used")
set(CACHE_OPTION_VALUES "ccache" "sccache")
set_property(CACHE CACHE_OPTION PROPERTY STRINGS ${CACHE_OPTION_VALUES})
list(FIND CACHE_OPTION_VALUES ${CACHE_OPTION} CACHE_OPTION_INDEX)

if(${CACHE_OPTION_INDEX} EQUAL -1)
  message(
    STATUS
      "Using custom compiler cache system: '${CACHE_OPTION}', explicitly supported entries are ${CACHE_OPTION_VALUES}"
  )
endif()

find_program(CACHE_BINARY ${CACHE_OPTION})
if(CACHE_BINARY)
  message(STATUS "${CACHE_OPTION} found and enabled")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CACHE_BINARY})
else()
  message(WARNING "${CACHE_OPTION} is enabled but was not found. Not using it")
endif()
