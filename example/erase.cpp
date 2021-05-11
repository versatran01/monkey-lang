#include <absl/strings/str_join.h>
#include <absl/types/optional.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include <iostream>
#include <string>
#include <vector>

#include "monkey/te.hpp"

namespace monkey {

enum class NodeType {
  kInvalid,
  kProgram,
  // Expression
  kIdentifier,
  kIntLiteral,
  kBoolLiteral,
  kPrefixExpr,
  kInfixExpr,
  kIfExpr,
  kFnLiteral,
  kCallExpr,
  // Statement
  kExprStmt,
  kLetStmt,
  kReturnStmt,
  kBlockStmt
};

std::ostream& operator<<(std::ostream& os, NodeType type);

/// Base Node
struct NodeBase {
  //  NodeBase() = default;
  explicit NodeBase(NodeType type) : type_{type} {}
  virtual ~NodeBase() noexcept = default;
  //  NodeBase(const NodeBase&) = default;
  //  NodeBase& operator=(const NodeBase&) = default;
  //  NodeBase(NodeBase&&) = default;
  //  NodeBase& operator=(NodeBase&&) = default;

  NodeType Type() const noexcept { return type_; }
  bool Ok() const noexcept { return type_ != NodeType::kInvalid; }

  std::string String() const noexcept { return StringImpl(); }
  virtual std::string StringImpl() const { return ""; }

 private:
  NodeType type_{NodeType::kInvalid};
};

/// Interface of Node
struct NodeInterface {
  auto String() const {
    return boost::te::call<std::string>(
        [](const auto& self) { return self.String(); }, *this);
  }

  auto Type() const noexcept {
    return boost::te::call<NodeType>(
        [](const auto& self) { return self.Type(); }, *this);
  }
};

using AstNode = boost::te::poly<NodeInterface>;
using StmtNode = AstNode;
using ExprNode = AstNode;

struct NodeFmt {
  void operator()(std::string* out, const AstNode& node) const {
    out->append(node.String());
  }
};

struct Program final : public NodeBase {
  Program() : NodeBase{NodeType::kProgram} {}
  std::string StringImpl() const override {
    return absl::StrJoin(statements, "\n", NodeFmt{});
  }

  std::vector<StmtNode> statements;
};

/// Expressions
struct Identifier final : public NodeBase {
  Identifier(const std::string& value)
      : NodeBase{NodeType::kIdentifier}, value{value} {}
  std::string StringImpl() const override { return value; }

  std::string value;
};

/// Statements
struct LetStatement final : public NodeBase {
  LetStatement(const std::string& name, const ExprNode& expr)
      : NodeBase{NodeType::kLetStmt}, name{name}, expr{expr} {}
  std::string StringImpl() const override {
    return fmt::format("let {} = {};", name.String(), expr.String());
  }

  Identifier name;
  ExprNode expr;
};

}  // namespace monkey

using namespace monkey;

int main() {
  Program program;
  Identifier ident{"v2"};
  LetStatement let{"v1", ident};

  program.statements.push_back(let);
  LOG(INFO) << ident.String();
  LOG(INFO) << let.String();
  LOG(INFO) << program.String();
  //  auto stmt = program.statements.front();
  //  LOG(INFO) << stmt.String();
  //  LOG(INFO) << program.String();
}
