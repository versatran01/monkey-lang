cc_library(
  NAME base
  DEPS fmt::fmt glog::glog monkey_warnings
  INTERFACE)

cc_library(
  NAME timer
  SRCS "timer.cpp"
  DEPS absl::time absl::strings absl::flat_hash_map monkey::base)

cc_library(
  NAME token
  SRCS "token.cpp"
  DEPS monkey::base absl::strings
  LINKOPTS absl::flat_hash_map)

cc_library(
  NAME lexer
  SRCS "lexer.cpp"
  DEPS monkey::token)

cc_library(
  NAME ast
  SRCS "ast.cpp"
  DEPS monkey::token
  LINKOPTS absl::strings absl::flat_hash_map)

cc_library(
  NAME parser
  SRCS "parser.cpp"
  DEPS monkey::ast monkey::token monkey::lexer absl::flat_hash_map
  LINKOPTS absl::strings)

cc_library(
  NAME code
  SRCS "code.cpp" "instruction.cpp"
  LINKOPTS absl::flat_hash_map monkey::base)

cc_library(
  NAME object
  SRCS "object.cpp"
  DEPS monkey::ast monkey::code absl::hash absl::flat_hash_map)

cc_library(
  NAME builtin
  SRCS "builtin.cpp"
  DEPS monkey::object)

cc_library(
  NAME environment
  SRCS "environment.cpp"
  DEPS monkey::object absl::flat_hash_map
  LINKOPTS absl::strings)

cc_library(
  NAME evaluator
  SRCS "evaluator.cpp"
  DEPS monkey::ast monkey::object monkey::builtin monkey::environment
       monkey::timer
  LINKOPTS absl::strings)

cc_library(
  NAME symbol
  SRCS "symbol.cpp"
  DEPS absl::flat_hash_map monkey::base)

cc_library(
  NAME compiler
  SRCS "compiler.cpp"
  DEPS monkey::code monkey::ast monkey::object monkey::symbol absl::statusor
  LINKOPTS monkey::timer)

cc_library(
  NAME vm
  SRCS "vm.cpp"
  DEPS monkey::compiler monkey::object monkey::timer)
