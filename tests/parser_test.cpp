#include "monkey/parser.h"

#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(ParserTest, TestParseProgram) {
  const std::string input = R"raw(
    let x = 5;
    let y = 10;
    let foobar =123;
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();
  ASSERT_EQ(program.statements.size(), 3);

    std::vector<std::string> expected_idents = {"x", "y", "foobar"};
    for (size_t i = 0; i < expected_idents.size(); ++i) {
      EXPECT_EQ(program.statements[i].TokenLiteral(), "let");
    }
}

}  // namespace
}  // namespace monkey
