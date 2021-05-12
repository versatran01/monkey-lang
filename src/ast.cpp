#include "monkey/ast.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/core.h>

namespace monkey {

namespace {

struct StatementFormatter {
  void operator()(std::string* out, const StmtNode& stmt) const {
    out->append(stmt.String());
  }
};

struct ExpressionFormatter {
  void operator()(std::string* out, const ExprNode& expr) const {
    out->append(expr.String());
  }
};

const auto gNodeTypeStrings = absl::flat_hash_map<NodeType, std::string>{
    {NodeType::kInvalid, "Invalid"},
    {NodeType::kProgram, "Program"},
    {NodeType::kIdentifier, "Identifier"},
    {NodeType::kIntLiteral, "IntLiteral"},
    {NodeType::kBoolLiteral, "BoolLiteral"},
    {NodeType::kPrefixExpr, "PrefixExpr"},
    {NodeType::kInfixExpr, "InfixExpr"},
    {NodeType::kIfExpr, "IfExpr"},
    {NodeType::kFnLiteral, "FuncLiteral"},
    {NodeType::kCallExpr, "CallExpr"},
    {NodeType::kExprStmt, "ExprStmt"},
    {NodeType::kLetStmt, "LetStmt"},
    {NodeType::kReturnStmt, "ReturnStmt"},
    {NodeType::kBlockStmt, "BlockStmt"}};
}  // namespace

std::ostream& operator<<(std::ostream& os, NodeType type) {
  return os << gNodeTypeStrings.at(type);
}

std::string Program::TokenLiteral() const {
  return statements.empty() ? "" : statements.front().TokenLiteral();
}

std::string Program::String() const {
  return absl::StrJoin(statements, "\n", StatementFormatter());
}

std::string LetStatement::String() const {
  return fmt::format(
      "{} {} = {};", TokenLiteral(), name.String(), expr.String());
}

std::string ReturnStatement::String() const {
  std::string str = TokenLiteral();
  if (expr.Ok()) {
    str += " " + expr.String();
  }
  str += ";";
  return str;
}

std::string PrefixExpression::String() const {
  return fmt::format("({}{})", op, rhs.String());
}

std::string InfixExpression::String() const {
  return fmt::format("({} {} {})", lhs.String(), op, rhs.String());
}

std::string IfExpression::String() const {
  std::string str;
  str += fmt::format("if {} {}", cond.String(), true_block.String());
  if (false_block.Ok() && false_block.size() > 0) {
    str += " else " + false_block.String();
  }
  return str;
}

std::string BlockStatement::String() const {
  return fmt::format("{}",
                     absl::StrJoin(statements, "; ", StatementFormatter()));
}

std::string FunctionLiteral::String() const {
  return fmt::format("{}({}) {{ {} }}",
                     TokenLiteral(),
                     absl::StrJoin(params, ", ", ExpressionFormatter()),
                     body.String());
}

std::string CallExpression::String() const {
  return fmt::format("{}({})",
                     func.String(),
                     absl::StrJoin(args, ", ", ExpressionFormatter()));
}

}  // namespace monkey
