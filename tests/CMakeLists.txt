find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(monkey_test lexer_test.cpp)
target_link_libraries(monkey_test PRIVATE monkey GTest::GTest GTest::Main)
add_test(NAME monkey_test COMMAND $<TARGET_FILE:monkey_test>)
set_tests_properties(monkey_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                            ".*FAILED.*")
