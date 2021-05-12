#include "monkey/ast.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(AstTest, TestAstNodeType) {
  EXPECT_EQ(Identifier().Type(), NodeType::kIdentifier);
  EXPECT_EQ(IntLiteral().Type(), NodeType::kIntLiteral);
  EXPECT_EQ(BoolLiteral().Type(), NodeType::kBoolLiteral);
  EXPECT_EQ(PrefixExpr().Type(), NodeType::kPrefixExpr);
  EXPECT_EQ(ExprStmt().Type(), NodeType::kExprStmt);
}

TEST(AstTest, TestConstruction) {
  const std::string name = "abc";
  Identifier expr;
  expr.token = Token{TokenType::kIdent, name};
  expr.value = name;

  ExprNode node = expr;
  EXPECT_EQ(node.String(), name);
  EXPECT_EQ(node.TokenLiteral(), name);
}

TEST(AstTest, TestCopyAndAssign) {
  const std::string v1 = "v1";
  const std::string v2 = "v2";
  Identifier ident;
  ident.token = Token{TokenType::kIdent, v2};
  ident.value = v2;
  EXPECT_EQ(ident.String(), v2);

  LetStmt let;
  let.token = Token{TokenType::kIdent, "let"};
  let.name.token = Token{TokenType::kIdent, v1};
  let.name.value = v1;
  let.expr = ident;
  EXPECT_EQ(let.expr.String(), v2);
  EXPECT_EQ(let.String(), "let v1 = v2;");

  std::vector<AstNode> nodes;
  nodes.push_back(let);
  EXPECT_EQ(ident.String(), v2);
  EXPECT_EQ(let.expr.String(), v2);
  EXPECT_EQ(let.String(), "let v1 = v2;");
  EXPECT_EQ(nodes.front().String(), "let v1 = v2;");

  AstNode copy = nodes.front();
  EXPECT_EQ(copy.String(), "let v1 = v2;");
  EXPECT_EQ(nodes.front().String(), "let v1 = v2;");
}

TEST(AstTest, TestPtrCast) {
  IntLiteral intl;
  intl.token = Token{TokenType::kInt, "5"};
  intl.value = 5;

  ExprNode expr = intl;
  const auto* ptr = expr.PtrCast<IntLiteral>();
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->value, intl.value);

  const auto* bad_ptr = expr.PtrCast<BoolLiteral>();
  ASSERT_EQ(bad_ptr, nullptr);
}

TEST(AstTest, TestExpressionStatement) {
  Identifier ident;
  ident.token = Token{TokenType::kIdent, "abc"};
  ident.value = "abc";

  ExprStmt stmt;
  stmt.token = ident.token;
  stmt.expr = ident;

  StmtNode node = stmt;
  EXPECT_EQ(node.TokenLiteral(), ident.TokenLiteral());
  const auto* ptr = node.PtrCast<ExprStmt>();
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->expr.String(), ident.String());
  EXPECT_EQ(ptr->expr.Type(), ident.Type());
}

TEST(AstTest, TestProgramString) {
  Program program;
  LetStmt let;
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
