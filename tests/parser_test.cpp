#include "monkey/parser.h"

#include <absl/types/variant.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

using LiteralType = absl::variant<bool, int, std::string>;

struct FuncTest {
  std::string input;
  std::vector<std::string> params;
};

struct LetTest {
  std::string input;
  std::string ident;
  LiteralType value;
};

struct PrefixTest {
  std::string input;
  std::string op;
  LiteralType rhs;
};

struct InfixTest {
  std::string input;
  LiteralType lhs;
  std::string op;
  LiteralType rhs;
};

/// Helper functions
void CheckIdentifier(const ExprNode& expr, const std::string& value) {
  ASSERT_EQ(expr.Type(), NodeType::kIdentifier);
  EXPECT_EQ(expr.TokenLiteral(), value);
  EXPECT_EQ(expr.String(), value);
  EXPECT_EQ(expr.PtrCast<Identifier>()->value, value);
}

void CheckIntLiteral(const ExprNode& expr, int64_t value) {
  ASSERT_EQ(expr.Type(), NodeType::kIntLiteral);
  EXPECT_EQ(expr.TokenLiteral(), std::to_string(value));
  EXPECT_EQ(expr.PtrCast<IntLiteral>()->value, value);
}

void CheckBoolLiteral(const ExprNode& expr, bool value) {
  ASSERT_EQ(expr.Type(), NodeType::kBoolLiteral);
  EXPECT_EQ(expr.TokenLiteral(), value ? "true" : "false");
  EXPECT_EQ(expr.PtrCast<BoolLiteral>()->value, value);
}

void CheckLetStatement(const StmtNode& stmt, const std::string& name) {
  ASSERT_EQ(stmt.TokenLiteral(), "let");
  ASSERT_EQ(stmt.Type(), NodeType::kLetStmt);
  CheckIdentifier(stmt.PtrCast<LetStmt>()->name, name);
}

void CheckLiteralExpression(const ExprNode& expr, const LiteralType& value) {
  switch (value.index()) {
    case 0:
      CheckBoolLiteral(expr, std::get<0>(value));
      break;
    case 1:
      CheckIntLiteral(expr, std::get<1>(value));
      break;
    case 2:
      CheckIdentifier(expr, std::get<2>(value));
      break;
    default:
      ASSERT_FALSE(true) << "Should not reach here, index: " << value.index();
  }
}

void CheckPrefixExpression(const ExprNode& expr,
                           const std::string& op,
                           const LiteralType& rhs) {
  ASSERT_EQ(expr.Type(), NodeType::kPrefixExpr);
  const auto* ptr = expr.PtrCast<PrefixExpr>();
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->op, op);
  CheckLiteralExpression(ptr->rhs, rhs);
}

void CheckInfixExpression(const ExprNode& expr,
                          const LiteralType& lhs,
                          const std::string& op,
                          const LiteralType& rhs) {
  ASSERT_EQ(expr.Type(), NodeType::kInfixExpr);
  const auto* ptr = expr.PtrCast<InfixExpr>();
  ASSERT_NE(ptr, nullptr);
  CheckLiteralExpression(ptr->lhs, lhs);
  EXPECT_EQ(ptr->op, op);
  CheckLiteralExpression(ptr->rhs, rhs);
}

/// Tests
TEST(ParserTest, TestParsingLetStatement) {
  const std::vector<LetTest> tests = {{"let x = 5;", "x", 5},
                                      {"let y = true;", "y", true},
                                      {"let z = y;", "z", std::string{"y"}}};

  for (const auto& test : tests) {
    Parser parser{test.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatements(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kLetStmt);
    CheckLetStatement(stmt, test.ident);
    CheckLiteralExpression(GetExpr(stmt), test.value);
  }
}

TEST(ParserTest, TestParsingReturnStatement) {
  const std::string input = R"raw(
    return 5;
    return 10;
    return 123;
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();

  ASSERT_EQ(program.NumStatements(), 3);
  for (const auto& stmt : program.statements) {
    EXPECT_EQ(stmt.TokenLiteral(), "return");
  }
}

TEST(ParserTest, TestParsingIdentExpression) {
  const std::string input = "foobar";
  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatements(), 1);

  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);

  const auto& expr = GetExpr(stmt);
  EXPECT_EQ(expr.TokenLiteral(), input);
  EXPECT_EQ(expr.String(), input);
}

TEST(ParserTest, TestIntLiteralExpression) {
  const std::string input = "5";
  Parser parser{input};
  const auto program = parser.ParseProgram();

  ASSERT_EQ(program.NumStatements(), 1);
  const auto stmt = program.statements.front();
  EXPECT_EQ(stmt.Type(), NodeType::kExprStmt);
  CheckIntLiteral(GetExpr(stmt), 5);
}

TEST(ParserTest, TestParsingBooleanExpression) {
  const std::string input = "true";
  Parser parser{input};
  const auto program = parser.ParseProgram();

  ASSERT_EQ(program.NumStatements(), 1);
  const auto stmt = program.statements.front();
  EXPECT_EQ(stmt.Type(), NodeType::kExprStmt);
  CheckBoolLiteral(GetExpr(stmt), true);
}

TEST(ParserTest, TestParsingPrefixExpressionInt) {
  const std::vector<PrefixTest> tests = {{"!5", "!", 5},
                                         {"-15", "-", 15},
                                         {"!true", "!", true},
                                         {"!false", "!", false}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    Parser parser{test.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatements(), 1) << parser.ErrorMsg();
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    CheckPrefixExpression(GetExpr(stmt), test.op, test.rhs);
  }
}

TEST(ParserTest, TestParsingInfixExpressionInt) {
  const std::vector<InfixTest> tests = {{"5+5;", 5, "+", 5},
                                        {"5-5;", 5, "-", 5},
                                        {"5*5;", 5, "*", 5},
                                        {"5/5;", 5, "/", 5},
                                        {"5>5;", 5, ">", 5},
                                        {"5<5;", 5, "<", 5},
                                        {"5>=5;", 5, ">=", 5},
                                        {"5<=5;", 5, "<=", 5},
                                        {"5==5;", 5, "==", 5},
                                        {"5!=5;", 5, "!=", 5},
                                        {"true==true;", true, "==", true},
                                        {"true!=false;", true, "!=", false},
                                        {"false==false;", false, "==", false}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    Parser parser{test.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatements(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    CheckInfixExpression(GetExpr(stmt), test.lhs, test.op, test.rhs);
  }
}

TEST(ParserTest, TestParsingOperatorPrecedence) {
  const std::vector<std::pair<std::string, std::string>> tests = {
      {"true", "true"},
      {"false", "false"},
      {"3 > 5 == false", "((3 > 5) == false)"},
      {"3 < 5 == true", "((3 < 5) == true)"},
      {"1+ (2+3) + 4", "((1 + (2 + 3)) + 4)"},
      {"a + add(b * c) + d", "((a + add((b * c))) + d)"},
      {"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
       "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
      {"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    Parser parser{test.first};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatements(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    EXPECT_EQ(stmt.PtrCast<ExprStmt>()->String(), test.second);
  }
}

TEST(ParserTest, TestParsingIfExpression) {
  const std::string input = "if (x < y) { x }";

  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatements(), 1);
  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
  const auto& expr = GetExpr(stmt);
  ASSERT_EQ(expr.Type(), NodeType::kIfExpr);
  const auto* ptr = expr.PtrCast<IfExpr>();
  ASSERT_NE(ptr, nullptr);
  CheckInfixExpression(
      ptr->cond, std::string{"x"}, std::string{"<"}, std::string{"y"});
  ASSERT_EQ(ptr->true_block.size(), 1);
  ASSERT_EQ(ptr->true_block.Type(), NodeType::kBlockStmt);
  const auto true_stmt = ptr->true_block.statements.front();
  ASSERT_EQ(true_stmt.Type(), NodeType::kExprStmt);
  CheckIdentifier(GetExpr(true_stmt), "x");
}

TEST(ParserTest, TestParsingFunctionLiteral) {
  const std::string input = "fn(x, y) { x + y; }";

  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatements(), 1);
  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
  const auto& expr = GetExpr(stmt);
  ASSERT_EQ(expr.Type(), NodeType::kFnLiteral);
  const auto* ptr = expr.PtrCast<FuncLiteral>();
  ASSERT_NE(ptr, nullptr);
  ASSERT_EQ(ptr->NumParams(), 2);
  CheckLiteralExpression(ptr->params[0], std::string("x"));
  CheckLiteralExpression(ptr->params[1], std::string("y"));
  ASSERT_EQ(ptr->body.size(), 1);
  const auto body_stmt = ptr->body.statements.front();
  CheckInfixExpression(
      GetExpr(body_stmt), std::string("x"), std::string("+"), std::string("y"));
}

TEST(ParserTest, TestParsingFunctionLiteral2) {
  const std::vector<FuncTest> tests = {{"fn() {};", {}},
                                       {"fn(x) {};", {"x"}},
                                       {"fn(x, y, z) {};", {"x", "y", "z"}}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    Parser parser{test.input};
    const auto program = parser.ParseProgram();
    ASSERT_EQ(program.NumStatements(), 1);
    const auto stmt = program.statements.front();
    ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
    const auto& expr = GetExpr(stmt);
    ASSERT_EQ(expr.Type(), NodeType::kFnLiteral);
    const auto* ptr = expr.PtrCast<FuncLiteral>();
    ASSERT_NE(ptr, nullptr);
    ASSERT_EQ(ptr->NumParams(), test.params.size());
    for (size_t i = 0; i < test.params.size(); ++i) {
      CheckIdentifier(ptr->params[i], test.params[i]);
    }
  }
}

TEST(ParserTest, TestParsingCallExpression) {
  const std::string input = "add(1, 2 * 3, 4 + 5);";
  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatements(), 1);
  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
  const auto& expr = GetExpr(stmt);
  ASSERT_EQ(expr.Type(), NodeType::kCallExpr);
  const auto* ptr = expr.PtrCast<CallExpr>();
  CheckIdentifier(ptr->func, "add");
  ASSERT_EQ(ptr->NumArgs(), 3);
  CheckLiteralExpression(ptr->args[0], 1);
  CheckInfixExpression(ptr->args[1], 2, "*", 3);
  CheckInfixExpression(ptr->args[2], 4, "+", 5);
}

TEST(ParserTest, TestStrLiteralExpression) {
  const std::string input = R"raw("hello world";)raw";
  Parser parser{input};
  const auto program = parser.ParseProgram();
  ASSERT_EQ(program.NumStatements(), 1);
  const auto stmt = program.statements.front();
  ASSERT_EQ(stmt.Type(), NodeType::kExprStmt);
  const auto& expr = GetExpr(stmt);
  ASSERT_EQ(expr.Type(), NodeType::kStrLiteral);
  const auto* ptr = expr.PtrCast<StrLiteral>();
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->value, "hello world");
}

}  // namespace
}  // namespace monkey
