#include "monkey/ast.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(AstTest, TestExpression) {
  Expression expr;
  EXPECT_EQ(expr.type, NodeType::kExpression);

  Identifier ident;
  EXPECT_EQ(ident.type, NodeType::kExpression);
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
  program.statements.push_back(let);

  EXPECT_EQ(program.String(), "let v1 = v2;\n");
}

}  // namespace
}  // namespace monkey
