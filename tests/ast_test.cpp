#include "monkey/ast.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(AstTest, TestExpressionType) {
  Expression expr;
  EXPECT_EQ(expr.Type(), NodeType::kBase);

  Identifier ident;
  EXPECT_EQ(ident.Type(), NodeType::kIdentExpr);
}

TEST(AstTest, TestExpressionValue) {
  Identifier expr;
  expr.type = NodeType::kIdentExpr;
  expr.token = Token{TokenType::kIdent, "abc"};
  expr.value = "abc";

  ExprNode node = expr;
  EXPECT_EQ(node.String(), expr.value);
  EXPECT_EQ(node.TokenLiteral(), expr.token.literal);
}

TEST(AstTest, TestStatementType) {
  Statement stmt;
  EXPECT_EQ(stmt.Type(), NodeType::kBase);

  LetStatement let;
  EXPECT_EQ(let.Type(), NodeType::kLetStmt);

  ReturnStatement ret;
  EXPECT_EQ(ret.Type(), NodeType::kReturnStmt);

  ExpressionStatement expr;
  EXPECT_EQ(expr.Type(), NodeType::kExprStmt);
}

TEST(AstTest, TestExpressionStatement) {
  Identifier expr;
  expr.type = NodeType::kIdentExpr;
  expr.token = Token{TokenType::kIdent, "abc"};
  expr.value = "abc";

  ExpressionStatement stmt;
  stmt.type = NodeType::kExprStmt;
  stmt.token = expr.token;
  stmt.expr = expr;

  StmtNode node = stmt;
  EXPECT_EQ(node.TokenLiteral(), expr.TokenLiteral());
  EXPECT_EQ(node.Expr().String(), expr.String());
  EXPECT_EQ(node.Expr().Type(), expr.Type());
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
  EXPECT_EQ(program.NumStatments(), 1);
  EXPECT_EQ(program.String(), "let v1 = v2;\n");
}

}  // namespace
}  // namespace monkey
