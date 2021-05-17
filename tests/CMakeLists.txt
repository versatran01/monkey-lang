find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

cc_gtest(
  NAME lexer_test
  SRCS "lexer_test.cpp"
  DEPS monkey::lexer)

cc_gtest(
  NAME ast_test
  SRCS "ast_test.cpp"
  DEPS monkey::ast)

cc_gtest(
  NAME parser_test
  SRCS "parser_test.cpp"
  DEPS monkey::parser)

cc_gtest(
  NAME object_test
  SRCS "object_test.cpp"
  DEPS monkey::object)

cc_gtest(
  NAME environment_test
  SRCS "environment_test.cpp"
  DEPS monkey::environment)

cc_gtest(
  NAME evaluator_test
  SRCS "evaluator_test.cpp"
  DEPS monkey::evaluator monkey::parser)

cc_gtest(
  NAME code_test
  SRCS "code_test.cpp"
  DEPS monkey::code GMock::GMock)

cc_gtest(
  NAME compiler_test
  SRCS "compiler_test.cpp"
  DEPS monkey::compiler monkey::parser GMock::GMock)

cc_gtest(
  NAME vm_test
  SRCS "vm_test.cpp"
  DEPS monkey::vm monkey::parser)
