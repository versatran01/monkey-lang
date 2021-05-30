find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

cc_test(
  NAME timer_test
  SRCS "timer_test.cpp"
  DEPS monkey::timer)

cc_test(
  NAME lexer_test
  SRCS "lexer_test.cpp"
  DEPS monkey::lexer)

cc_test(
  NAME ast_test
  SRCS "ast_test.cpp"
  DEPS monkey::ast)

cc_test(
  NAME parser_test
  SRCS "parser_test.cpp"
  DEPS monkey::parser)

cc_test(
  NAME object_test
  SRCS "object_test.cpp"
  DEPS monkey::object)

cc_test(
  NAME environment_test
  SRCS "environment_test.cpp"
  DEPS monkey::environment)

cc_test(
  NAME evaluator_test
  SRCS "evaluator_test.cpp"
  DEPS monkey::evaluator monkey::parser)

cc_test(
  NAME code_test
  SRCS "code_test.cpp"
  DEPS monkey::code GMock::GMock)

cc_test(
  NAME symbol_test
  SRCS "symbol_test.cpp"
  DEPS monkey::symbol)

cc_test(
  NAME compiler_test
  SRCS "compiler_test.cpp"
  DEPS monkey::compiler monkey::parser GMock::GMock)

cc_test(
  NAME vm_test
  SRCS "vm_test.cpp"
  DEPS monkey::vm monkey::parser GMock::GMock)
