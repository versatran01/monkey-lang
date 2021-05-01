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

  ASSERT_EQ(program.NumStatments(), expected_idents.size());
  for (const auto& stmt : program.statements) {
    EXPECT_EQ(stmt.TokenLiteral(), "let");
  }
}

TEST(ParserTest, TestParseLetStatementWithError) {
  const std::string input = R"raw(
    let x = 5;
    let y = 10;
    let 123;
   )raw";

  Parser parser{input};

  const auto program = parser.ParseProgram();
  const std::vector<std::string> expected_idents = {"x", "y"};
  ASSERT_EQ(program.NumStatments(), expected_idents.size());

  for (const auto& stmt : program.statements) {
    EXPECT_EQ(stmt.TokenLiteral(), "let");
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

  ASSERT_EQ(program.NumStatments(), 3);
  for (const auto& stmt : program.statements) {
    EXPECT_EQ(stmt.TokenLiteral(), "return");
  }
}

//TEST(ParserTest, TestIdentifierExpression) {
//  const std::string input = "foobar";
//  Parser parser{input};
//  const auto program = parser.ParseProgram();
//  ASSERT_EQ(program.NumStatments(), 1);

//  const auto stmt = program.statements.front();
//  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);

//  const auto expr = stmt.Expr();
//  EXPECT_EQ(expr.TokenLiteral(), input);
//  EXPECT_EQ(expr.String(), input);
//}

}  // namespace
}  // namespace monkey
