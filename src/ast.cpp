#include "monkey/ast.h"

#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>

namespace monkey {

namespace {

const auto gNodeTypeStrings = absl::flat_hash_map<NodeType, std::string>{
    {NodeType::kInvalid, "Invalid"},
    {NodeType::kProgram, "Program"},
    {NodeType::kIdentifier, "Identifier"},
    {NodeType::kIntLiteral, "IntegerLiteral"},
    {NodeType::kBoolLiteral, "BoolLIteral"},
    {NodeType::kPrefixExpr, "PrefixExpr"},
    {NodeType::kInfixExpr, "InfixExpr"},
    {NodeType::kIfExpr, "IfExpr"},
    {NodeType::kExprStmt, "ExprStmt"},
    {NodeType::kLetStmt, "LetStmt"},
    {NodeType::kReturnStmt, "ReturnStmt"},
    {NodeType::kBlockStmt, "BlockStmt"}};
}

std::ostream& operator<<(std::ostream& os, NodeType type) {
  return os << gNodeTypeStrings.at(type);
}

std::string Program::TokenLiteralImpl() const {
  return statements.empty() ? "" : statements.front().TokenLiteral();
}

std::string Program::StringImpl() const {
  std::string res;
  for (const auto& stmt : statements) {
    res += stmt.String() + "\n";
  }
  return res;
}

std::string LetStatement::StringImpl() const {
  return fmt::format("{} {} = {};", TokenLiteral(), name.String(),
                     expr.String());
}

std::string ReturnStatement::StringImpl() const {
  return fmt::format("{} {};", TokenLiteral(), expr.String());
}

std::string PrefixExpression::StringImpl() const {
  return fmt::format("({}{})", op, rhs.String());
}

std::string InfixExpression::StringImpl() const {
  return fmt::format("({} {} {})", lhs.String(), op, rhs.String());
}

std::string ExpressionStatement::StringImpl() const {
  return fmt::format("{};", expr.String());
}

std::string IfExpression::StringImpl() const {
  std::string str;
  str += fmt::format("if {} {}", cond.String(), true_stmt.String());
  if (false_stmt.Ok()) {
    str += "else " + false_stmt.String();
  }
  return str;
}

std::string BlockStatement::StringImpl() const {
  std::string str = "{";
  const auto n = statements.size();
  for (size_t i = 0; i < n; ++i) {
    str += statements[i].String();
    if (i < n - 1) str += " ";
  }
  str += "}";
  return str;
}

}  // namespace monkey
