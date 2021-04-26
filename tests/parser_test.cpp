#include "monkey/parser.h"

#include <fmt/ranges.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(ParserTest, TestParseLetStatement) {
  const std::string input = R"raw(
    let x = 5;
    let y = 10;
    let foobar =123;
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();
  const std::vector<std::string> expected_idents = {"x", "y", "foobar"};
  ASSERT_EQ(program.statements.size(), expected_idents.size());

  for (size_t i = 0; i < program.statements.size(); ++i) {
    EXPECT_EQ(program.statements[i].TokenLiteral(), "let");
  }
}

TEST(ParserTest, TestParseLetStatementWithError) {
  const std::string input = R"raw(
    let x = 5;
    let y = 10;
    let 123;
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();
  const std::vector<std::string> expected_idents = {"x", "y"};
  ASSERT_EQ(program.statements.size(), expected_idents.size());

  for (size_t i = 0; i < program.statements.size(); ++i) {
    EXPECT_EQ(program.statements[i].TokenLiteral(), "let");
  }
  LOG(INFO) << fmt::format("{}", parser.errors());
}

TEST(ParserTest, TestParseReturnStatement) {
  const std::string input = R"raw(
    return 5;
    return 10;
    return 123;
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();
  ASSERT_EQ(program.statements.size(), 3);

  for (size_t i = 0; i < program.statements.size(); ++i) {
    EXPECT_EQ(program.statements[i].TokenLiteral(), "return");
  }
}

}  // namespace
}  // namespace monkey
