#add_executable(monkey_play play.cpp)
#target_link_libraries(monkey_play PRIVATE monkey_parser monkey_evaluator)

add_executable(monkey_rlpl rlpl.cpp)
target_link_libraries(monkey_rlpl PRIVATE monkey_lexer)

#add_executable(monkey_rppl rppl.cpp)
#target_link_libraries(monkey_rppl PRIVATE monkey_parser)

#add_executable(monkey_repl repl.cpp)
#target_link_libraries(monkey_repl PRIVATE monkey_evaluator monkey_parser)

add_executable(erase erase3.cpp)
target_include_directories(erase PRIVATE ../include)
target_link_libraries(erase PRIVATE fmt::fmt glog::glog absl::strings)

