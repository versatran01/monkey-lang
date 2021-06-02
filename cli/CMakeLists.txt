cc_binary(
  NAME rlpl
  SRCS "rlpl.cpp"
  DEPS monkey::lexer monkey::base)

cc_binary(
  NAME rppl
  SRCS "rppl.cpp"
  DEPS monkey::parser)

cc_binary(
  NAME repl
  SRCS "repl.cpp"
  DEPS monkey::parser monkey::evaluator monkey::vm absl::flags
       absl::flags_parse)

cc_binary(
  NAME play
  SRCS "play.cpp"
  DEPS monkey::base monkey::parser monkey::evaluator monkey::environment)
