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
  ASSERT_EQ(program.NumStatments(), 3);

  for (size_t i = 0; i < expected_idents.size(); ++i) {
    EXPECT_EQ(program.statements[i].TokenLiteral(), "let");
  }
  LOG(INFO) << fmt::format("{}", parser.ErrorMsg());
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

TEST(ParserTest, TestIdentExpression) {
  const std::string input = "foobar";
  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatments(), 1);

  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);

  const auto expr = stmt.Expr();
  EXPECT_EQ(expr.TokenLiteral(), input);
  EXPECT_EQ(expr.String(), input);
}

TEST(ParserTest, TestIntLiteralExpression) {
  const std::string input = "5";
  Parser parser{input};
  const auto program = parser.ParseProgram();

  ASSERT_EQ(program.NumStatments(), 1);
  const auto stmt = program.statements.front();
  EXPECT_EQ(stmt.Type(), NodeType::kExprStmt);

  const auto expr = stmt.Expr();
  EXPECT_EQ(expr.Type(), NodeType::kIntLiteral);
  EXPECT_EQ(expr.TokenLiteral(), "5");
  auto* intl_ptr = dynamic_cast<IntegerLiteral*>(expr.Ptr());
  ASSERT_TRUE(intl_ptr != nullptr);
  EXPECT_EQ(intl_ptr->value, 5);
}

void TestIntegerLiteral(const Expression& expr, int64_t value) {
  const auto* ptr = dynamic_cast<IntegerLiteral*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->value, value);
  EXPECT_EQ(ptr->TokenLiteral(), std::to_string(value));
}

TEST(ParserTest, TestPrefixOperator) {
  struct Prefix {
    std::string input;
    std::string op;
    int64_t value;
  };

  std::vector<Prefix> prefixes = {{"!5", "!", 5}, {"-15", "-", 15}};
  for (const auto& prefix : prefixes) {
    Parser parser{prefix.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatments(), 1) << parser.ErrorMsg();
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    const auto expr = stmt.Expr();
    ASSERT_EQ(expr.Type(), NodeType::kPrefixExpr);
    const auto* ptr = dynamic_cast<PrefixExpression*>(expr.Ptr());
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->op, prefix.op);
    TestIntegerLiteral(ptr->rhs, prefix.value);
  }
}

}  // namespace
}  // namespace monkey
