add_library(monkey_core INTERFACE)
target_include_directories(monkey_core INTERFACE ../include)
target_link_libraries(monkey_core INTERFACE fmt::fmt glog::glog monkey_warnings)

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
  PUBLIC monkey_ast monkey_lexer
  PRIVATE absl::strings absl::flat_hash_map)

add_library(monkey_object object.cpp)
target_include_directories(monkey_object PUBLIC ../include)
target_link_libraries(
  monkey_object
  PUBLIC monkey_core
  PRIVATE absl::flat_hash_map)

add_library(monkey_eval evaluator.cpp)
target_link_libraries(monkey_eval PUBLIC monkey_object monkey_ast)

add_library(monkey_env environment.cpp)
target_link_libraries(monkey_env PUBLIC monkey_object absl::node_hash_map)
