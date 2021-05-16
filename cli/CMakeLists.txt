cc_binary(
  NAME rlpl
  SRCS "rlpl.cpp"
  DEPS monkey::lexer)

cc_binary(
  NAME rppl
  SRCS "rppl.cpp"
  DEPS monkey::parser)

cc_binary(
  NAME repl
  SRCS "repl.cpp"
  DEPS monkey::parser monkey::evaluator)

cc_binary(
  NAME play
  SRCS "play.cpp"
  DEPS monkey::base)
