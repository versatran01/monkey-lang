add_library(monkey_core INTERFACE)
target_include_directories(monkey_core INTERFACE ../include)
target_link_libraries(monkey_core INTERFACE fmt::fmt glog::glog monkey_warnings)

add_library(monkey_timer timer.cpp)
target_link_libraries(
  monkey_timer
  PUBLIC monkey_core
         absl::time
         absl::strings
         absl::flat_hash_map)

add_library(monkey_token token.cpp)
target_link_libraries(
  monkey_token
  PUBLIC monkey_core
  PRIVATE absl::flat_hash_map)

add_library(monkey_lexer lexer.cpp)
target_link_libraries(monkey_lexer PUBLIC monkey_token)

add_library(monkey_ast ast.cpp)
target_link_libraries(
  monkey_ast
  PUBLIC monkey_token
  PRIVATE absl::strings absl::flat_hash_map)

add_library(monkey_parser parser.cpp)
target_link_libraries(
  monkey_parser
  PUBLIC monkey_ast
         monkey_token
         monkey_lexer
         absl::flat_hash_map
  PRIVATE absl::strings)

add_library(monkey_object object.cpp)
target_link_libraries(monkey_object PUBLIC monkey_ast absl::hash
                                           absl::flat_hash_map)

add_library(monkey_builtin builtin.cpp)
target_link_libraries(monkey_builtin PUBLIC monkey_object)

add_library(monkey_environment environment.cpp)
target_link_libraries(
  monkey_environment
  PUBLIC monkey_object absl::flat_hash_map
  PRIVATE absl::strings)

add_library(monkey_evaluator evaluator.cpp)
target_link_libraries(
  monkey_evaluator
  PUBLIC monkey_ast
         monkey_object
         monkey_builtin
         monkey_environment
  PRIVATE absl::strings)
