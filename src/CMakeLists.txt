add_library(monkey_lexer token.cpp lexer.cpp)
target_include_directories(monkey_lexer PUBLIC ../include)
target_link_libraries(
  monkey_lexer
  PUBLIC glog::glog fmt::fmt
  PRIVATE absl::flat_hash_map)

add_library(monkey_parser ast.cpp parser.cpp)
target_link_libraries(
  monkey_parser
  PUBLIC monkey_lexer absl::flat_hash_map
  PRIVATE absl::strings)
