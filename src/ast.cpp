#include "monkey/ast.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/core.h>

namespace monkey {

namespace {

struct NodeFmt {
  void operator()(std::string* out, const AstNode& expr) const {
    out->append(expr.String());
  }
};

const auto gNodeTypeStrings = absl::flat_hash_map<NodeType, std::string>{
    {NodeType::kInvalid, "Invalid"},
    {NodeType::kProgram, "Program"},
    {NodeType::kIdentifier, "Identifier"},
    {NodeType::kIntLiteral, "IntLiteral"},
    {NodeType::kBoolLiteral, "BoolLiteral"},
    {NodeType::kArrayLiteral, "ArrayLiteral"},
    {NodeType::kHashLiteral, "HashLiteral"},
    {NodeType::kPrefixExpr, "PrefixExpr"},
    {NodeType::kInfixExpr, "InfixExpr"},
    {NodeType::kIfExpr, "IfExpr"},
    {NodeType::kIndexExpr, "IndexExpr"},
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
  return absl::StrJoin(statements, "\n", NodeFmt());
}

std::string LetStmt::String() const {
  return fmt::format(
      "{} {} = {};", TokenLiteral(), name.String(), expr.String());
}

std::string ReturnStmt::String() const {
  std::string str = TokenLiteral();
  if (expr.Ok()) {
    str += " " + expr.String();
  }
  str += ";";
  return str;
}

std::string ArrayLiteral::String() const {
  return fmt::format("[{}]", absl::StrJoin(elements, ", ", NodeFmt()));
}

std::string PrefixExpr::String() const {
  return fmt::format("({}{})", op, rhs.String());
}

std::string InfixExpr::String() const {
  return fmt::format("({} {} {})", lhs.String(), op, rhs.String());
}

std::string IfExpr::String() const {
  std::string str;
  str += fmt::format("if {} {}", cond.String(), true_block.String());
  if (false_block.Ok() && false_block.size() > 0) {
    str += " else " + false_block.String();
  }
  return str;
}

std::string BlockStmt::String() const {
  return fmt::format("{}", absl::StrJoin(statements, "; ", NodeFmt()));
}

std::string FuncLiteral::String() const {
  return fmt::format("{}({}) {{ {} }}",
                     TokenLiteral(),
                     absl::StrJoin(params,
                                   ", ",
                                   [](std::string* out, const Identifier& p) {
                                     out->append(p.String());
                                   }),
                     body.String());
}

std::string CallExpr::String() const {
  return fmt::format(
      "{}({})", func.String(), absl::StrJoin(args, ", ", NodeFmt()));
}

const ExprNode& GetExpr(const StmtNode& node) {
  switch (node.Type()) {
    case NodeType::kExprStmt:
      return node.PtrCast<ExprStmt>()->expr;
    case NodeType::kLetStmt:
      return node.PtrCast<LetStmt>()->expr;
    case NodeType::kReturnStmt:
      return node.PtrCast<ReturnStmt>()->expr;
    default:
      throw std::runtime_error(
          "GetExpr can only be called on ExprStmt, LetStmt and ReturnStmt");
  }
}

std::string IndexExpr::String() const {
  return fmt::format("({}[{}])", lhs.String(), index.String());
}

std::string HashLiteral::String() const {
  const auto pf = absl::PairFormatter(NodeFmt(), ": ", NodeFmt());
  return fmt::format("{{{}}}", absl::StrJoin(pairs, ", ", pf));
}

}  // namespace monkey
