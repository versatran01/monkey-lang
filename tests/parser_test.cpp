#include "monkey/parser.h"

#include <fmt/ranges.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

template <typename T>
struct Infix {
  std::string input;
  T lhs;
  std::string op;
  T rhs;
};

template <typename T>
struct Prefix {
  std::string input;
  std::string op;
  T rhs;
};

/// Helper functions
void CheckIdentifier(const Expression& expr, const std::string& value) {
  ASSERT_EQ(expr.Type(), NodeType::kIdentifier);
  const auto* ptr = dynamic_cast<Identifier*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->TokenLiteral(), value);
  EXPECT_EQ(ptr->value, value);
}

void CheckIntegerLiteral(const Expression& expr, int64_t value) {
  ASSERT_EQ(expr.Type(), NodeType::kIntLiteral);
  const auto* ptr = dynamic_cast<IntegerLiteral*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->value, value);
  EXPECT_EQ(ptr->TokenLiteral(), std::to_string(value));
}

void CheckBooleanLiteral(const Expression& expr, bool value) {
  ASSERT_EQ(expr.Type(), NodeType::kBoolLiteral);
  const auto* ptr = dynamic_cast<BooleanLiteral*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->value, value);
  EXPECT_EQ(ptr->TokenLiteral(), value ? "true" : "false");
}

template <typename T>
void CheckLiteralExpression(const Expression& expr, const T& v) {
  if constexpr (std::is_same_v<bool, T>) {
    CheckBooleanLiteral(expr, v);
  } else if (std::is_integral_v<T>) {
    CheckIntegerLiteral(expr, v);
  }
}

void CheckLiteralExpression(const Expression& expr, const std::string& v) {
  CheckIdentifier(expr, v);
}

template <typename T>
void CheckInfixExpression(const Expression& expr, const T& lhs,
                          const std::string& op, const T& rhs) {
  ASSERT_EQ(expr.Type(), NodeType::kInfixExpr);
  const auto* ptr = dynamic_cast<InfixExpression*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  CheckLiteralExpression(ptr->lhs, lhs);
  EXPECT_EQ(ptr->op, op);
  CheckLiteralExpression(ptr->rhs, rhs);
}

template <typename T>
void CheckPrefixExpression(const Expression& expr, const std::string& op,
                           const T& rhs) {
  ASSERT_EQ(expr.Type(), NodeType::kPrefixExpr);
  const auto* ptr = dynamic_cast<PrefixExpression*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->op, op);
  CheckLiteralExpression(ptr->rhs, rhs);
}

/// Tests
TEST(ParserTest, TestParsingLetStatement) {
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

TEST(ParserTest, TestParsingLetStatementWithError) {
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

TEST(ParserTest, TestParsingReturnStatement) {
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

TEST(ParserTest, TestParsingIdentExpression) {
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
  CheckIntegerLiteral(stmt.Expr(), 5);
}

TEST(ParserTest, TestParsingBooleanExpression) {
  const std::string input = "true";
  Parser parser{input};
  const auto program = parser.ParseProgram();

  ASSERT_EQ(program.NumStatments(), 1);
  const auto stmt = program.statements.front();
  EXPECT_EQ(stmt.Type(), NodeType::kExprStmt);
  CheckBooleanLiteral(stmt.Expr(), true);
}

TEST(ParserTest, TestParsingPrefixExpressionInt) {
  const std::vector<Prefix<int64_t>> prefixes = {{"!5", "!", 5},
                                                 {"-15", "-", 15}};
  for (const auto& prefix : prefixes) {
    Parser parser{prefix.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatments(), 1) << parser.ErrorMsg();
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    CheckPrefixExpression(stmt.Expr(), prefix.op, prefix.rhs);
  }
}

TEST(ParserTest, TestParsingPrefixExpressionBool) {
  const std::vector<Prefix<bool>> prefixes = {{"!true", "!", true},
                                              {"!false", "!", false}};
  for (const auto& prefix : prefixes) {
    Parser parser{prefix.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatments(), 1) << parser.ErrorMsg();
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    CheckPrefixExpression(stmt.Expr(), prefix.op, prefix.rhs);
  }
}

TEST(ParserTest, TestParsingInfixExpressionInt) {
  const std::vector<Infix<int64_t>> infixes = {
      {"5+5;", 5, "+", 5},   {"5-5;", 5, "-", 5},   {"5*5;", 5, "*", 5},
      {"5/5;", 5, "/", 5},   {"5>5;", 5, ">", 5},   {"5<5;", 5, "<", 5},
      {"5>=5;", 5, ">=", 5}, {"5<=5;", 5, "<=", 5}, {"5==5;", 5, "==", 5},
      {"5!=5;", 5, "!=", 5}};

  for (const auto& infix : infixes) {
    Parser parser{infix.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatments(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    CheckInfixExpression(stmt.Expr(), infix.lhs, infix.op, infix.rhs);
  }
}

TEST(ParserTest, TestParsingInfixExpressionBool) {
  const std::vector<Infix<bool>> infixes = {
      {"true==true;", true, "==", true},
      {"true!=false;", true, "!=", false},
      {"false==false;", false, "==", false}};

  for (const auto& infix : infixes) {
    Parser parser{infix.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatments(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    CheckInfixExpression(stmt.Expr(), infix.lhs, infix.op, infix.rhs);
  }
}

TEST(ParserTest, TestParsingOperatorPrecedence) {
  const std::vector<std::pair<std::string, std::string>> tests = {
      {"true", "true"},
      {"false", "false"},
      {"3 > 5 == false", "((3 > 5) == false)"},
      {"3 < 5 == true", "((3 < 5) == true)"},
      {"1+ (2+3) + 4", "((1 + (2 + 3)) + 4)"}};

  for (const auto& test : tests) {
    Parser parser{test.first};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatments(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    EXPECT_EQ(stmt.Expr().String(), test.second);
  }
}

TEST(ParserTest, TestParsingIfExpression) {
  const std::string input = "if (x < y) { x }";

  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatments(), 1);
  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
  const auto expr = stmt.Expr();
  ASSERT_EQ(expr.Type(), NodeType::kIfExpr);
  const auto* ptr = dynamic_cast<IfExpression*>(expr.Ptr());
  ASSERT_NE(ptr, nullptr);
  CheckInfixExpression(ptr->cond, std::string{"x"}, std::string{"<"},
                       std::string{"y"});
  ASSERT_EQ(ptr->true_block.NumStatements(), 1);
  ASSERT_EQ(ptr->true_block.Type(), NodeType::kBlockStmt);
  const auto true_expr = ptr->true_block.statements.front();
  ASSERT_EQ(true_expr.Type(), NodeType::kExprStmt);
  CheckIdentifier(true_expr.Expr(), "x");
}

}  // namespace
}  // namespace monkey
