find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_library(monkey_test INTERFACE)
target_link_libraries(monkey_test INTERFACE GTest::GTest GTest::Main)

add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE monkey_test monkey_lexer glog::glog)
add_test(NAME lexer_test COMMAND $<TARGET_FILE:lexer_test>)
set_tests_properties(lexer_test PROPERTIES FAIL_REGULAR_EXPRESSION ".*FAILED.*")

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE monkey_ast monkey_test)
add_test(NAME ast_test COMMAND $<TARGET_FILE:ast_test>)
set_tests_properties(ast_test PROPERTIES FAIL_REGULAR_EXPRESSION ".*FAILED.*")

add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test PRIVATE monkey_parser monkey_test)
add_test(NAME parser_test COMMAND $<TARGET_FILE:parser_test>)
set_tests_properties(parser_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                            ".*FAILED.*")

add_executable(object_test object_test.cpp)
target_link_libraries(object_test PRIVATE monkey_object monkey_test)
add_test(NAME object_test COMMAND $<TARGET_FILE:object_test>)
set_tests_properties(object_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                            ".*FAILED.*")

add_executable(environment_test environment_test.cpp)
target_link_libraries(environment_test PRIVATE monkey_environment monkey_test)
add_test(NAME environment_test COMMAND $<TARGET_FILE:environment_test>)
set_tests_properties(environment_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                                 ".*FAILED.*")

add_executable(evaluator_test evaluator_test.cpp)
target_link_libraries(evaluator_test PRIVATE monkey_evaluator monkey_parser
                                             monkey_test)
add_test(NAME evaluator_test COMMAND $<TARGET_FILE:evaluator_test>)
set_tests_properties(evaluator_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                               ".*FAILED.*")
