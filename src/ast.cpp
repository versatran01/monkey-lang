#include "monkey/ast.h"

#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>

namespace monkey {

namespace {

const auto gNodeTypeStrings = absl::flat_hash_map<NodeType, std::string>{
    {NodeType::kInvalid, "Invalid"},
    {NodeType::kProgram, "Program"},
    {NodeType::kIdentifier, "Identifier"},
    {NodeType::kIntLiteral, "Integer"},
    {NodeType::kBoolLiteral, "Bool"},
    {NodeType::kPrefixExpr, "Prefix"},
    {NodeType::kInfixExpr, "Infix"},
    {NodeType::kExprStmt, "ExprStmt"},
    {NodeType::kLetStmt, "LetStmt"},
    {NodeType::kReturnStmt, "ReturnStmt"},
};
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

}  // namespace monkey
