find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE monkey GTest::GTest GTest::Main)
add_test(NAME lexer_test COMMAND $<TARGET_FILE:lexer_test>)
set_tests_properties(lexer_test PROPERTIES FAIL_REGULAR_EXPRESSION ".*FAILED.*")

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE monkey GTest::GTest GTest::Main)
add_test(NAME ast_test COMMAND $<TARGET_FILE:ast_test>)
set_tests_properties(ast_test PROPERTIES FAIL_REGULAR_EXPRESSION ".*FAILED.*")

add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test PRIVATE monkey GTest::GTest GTest::Main)
add_test(NAME parser_test COMMAND $<TARGET_FILE:parser_test>)
set_tests_properties(parser_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                            ".*FAILED.*")
