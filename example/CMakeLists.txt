add_executable(monkey_play play.cpp)
target_link_libraries(monkey_play PRIVATE monkey_parser)

add_executable(monkey_rlpl rlpl.cpp)
target_link_libraries(monkey_rlpl PRIVATE monkey_lexer)

add_executable(monkey_rppl rppl.cpp)
target_link_libraries(monkey_rppl PRIVATE monkey_parser)

add_executable(monkey_repl repl.cpp)
target_link_libraries(monkey_repl PRIVATE monkey_eval)
