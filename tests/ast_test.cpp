#include "monkey/ast.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(AstTest, TestExpressionType) {
  ExpressionBase expr;
  EXPECT_EQ(expr.Type(), NodeType::kInvalid);

  Identifier ident;
  EXPECT_EQ(ident.Type(), NodeType::kIdentifier);
}

TEST(AstTest, TestExpressionValue) {
  Identifier expr;
  expr.type = NodeType::kIdentifier;
  expr.token = Token{TokenType::kIdent, "abc"};
  expr.value = "abc";

  Expression node = expr;
  EXPECT_EQ(node.String(), expr.value);
  EXPECT_EQ(node.TokenLiteral(), expr.token.literal);
}

TEST(AstTest, TestStatementType) {
  StatementBase stmt;
  EXPECT_EQ(stmt.Type(), NodeType::kInvalid);
  EXPECT_EQ(stmt.Ok(), false);

  LetStatement let;
  EXPECT_EQ(let.Type(), NodeType::kLetStmt);
  EXPECT_EQ(let.Ok(), true);

  ReturnStatement ret;
  EXPECT_EQ(ret.Type(), NodeType::kReturnStmt);
  EXPECT_EQ(ret.Ok(), true);

  ExpressionStatement expr;
  EXPECT_EQ(expr.Type(), NodeType::kExprStmt);
  EXPECT_EQ(expr.Ok(), true);
}

TEST(AstTest, TestExpressionStatement) {
  Identifier expr;
  expr.type = NodeType::kIdentifier;
  expr.token = Token{TokenType::kIdent, "abc"};
  expr.value = "abc";

  ExpressionStatement stmt;
  stmt.type = NodeType::kExprStmt;
  stmt.token = expr.token;
  stmt.expr = expr;

  Statement node = stmt;
  EXPECT_EQ(node.TokenLiteral(), expr.TokenLiteral());
  EXPECT_EQ(node.Expr().String(), expr.String());
  EXPECT_EQ(node.Expr().Type(), expr.Type());
}

TEST(AstTest, TestExpressionPtr) {
  IntegerLiteral intl;
  intl.type = NodeType::kIntLiteral;
  intl.token = Token{TokenType::kInt, "5"};
  intl.value = 5;

  Expression expr = intl;
  auto* base_ptr = expr.Ptr();
  auto* intl_ptr = dynamic_cast<IntegerLiteral*>(base_ptr);
  ASSERT_NE(intl_ptr, nullptr);
  EXPECT_EQ(intl_ptr->value, intl.value);

  auto* bad_ptr = dynamic_cast<Identifier*>(base_ptr);
  ASSERT_EQ(bad_ptr, nullptr);
}

TEST(AstTest, TestProgramString) {
  Program program;
  LetStatement let;
  let.token = Token{TokenType::kLet, "let"};
  let.name.token = Token{TokenType::kIdent, "v1"};
  let.name.value = "v1";

  Identifier ident;
  ident.token = Token{TokenType::kIdent, "v2"};
  ident.value = "v2";
  let.expr = ident;

  program.AddStatement(let);
  EXPECT_EQ(program.NumStatements(), 1);
  EXPECT_EQ(program.String(), "let v1 = v2;");
}

}  // namespace
}  // namespace monkey
