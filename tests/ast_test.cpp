#include "monkey/ast.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(AstTest, TestExpressionType) {
  NodeBase expr;
  EXPECT_EQ(expr.Type(), NodeType::kInvalid);

  Identifier ident;
  EXPECT_EQ(ident.Type(), NodeType::kIdentifier);
}

TEST(AstTest, TestExpressionValue) {
  Identifier expr;
  expr.token = Token{TokenType::kIdent, "abc"};
  expr.value = "abc";

  ExprNode node = expr;
  EXPECT_EQ(node.String(), expr.value);
  EXPECT_EQ(node.TokenLiteral(), expr.token.literal);
}

TEST(AstTest, TestStatementType) {
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
  Identifier ident;
  ident.token = Token{TokenType::kIdent, "abc"};
  ident.value = "abc";

  ExpressionStatement stmt;
  stmt.token = ident.token;
  stmt.expr = ident;

  StmtNode node = stmt;
  EXPECT_EQ(node.TokenLiteral(), ident.TokenLiteral());
  EXPECT_EQ(node.Expr().String(), ident.String());
  EXPECT_EQ(node.Expr().Type(), ident.Type());
}

TEST(AstTest, TestExpressionPtr) {
  IntLiteral intl;
  intl.token = Token{TokenType::kInt, "5"};
  intl.value = 5;

  ExprNode expr = intl;
  auto* base_ptr = expr.Ptr();
  auto* intl_ptr = dynamic_cast<IntLiteral*>(base_ptr);
  ASSERT_NE(intl_ptr, nullptr);
  EXPECT_EQ(intl_ptr->value, intl.value);
  EXPECT_EQ(expr.PtrCast<IntLiteral>()->value, intl.value);

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
