#include <absl/strings/str_join.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <string>
#include <vector>

#include "monkey/te.hpp"

enum class NodeType {
  kInvalid,
  kProgram,
  // Expression
  kIdentifier,
  // Statement
  kLetStmt,
};

std::ostream& operator<<(std::ostream& os, NodeType type);

/// Base Node
struct NodeBase {
  NodeBase() = default;
  explicit NodeBase(NodeType type) : type_{type} {}
  virtual ~NodeBase() noexcept = default;

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
struct NodeFmt {
  void operator()(std::string* out, const AstNode& node) const {
    out->append(node.String());
  }
};

/// Expressions
struct Identifier final : public NodeBase {
  Identifier() : NodeBase{NodeType::kIdentifier} {}
  std::string StringImpl() const override { return value; }

  std::string value;
};

/// Statements
struct LetStatement final : public NodeBase {
  LetStatement() : NodeBase{NodeType::kLetStmt} {}
  std::string StringImpl() const override {
    return fmt::format("let {} = {};", name.String(), expr.String());
  }

  Identifier name;
  AstNode expr{NodeBase{}};
};

int main() {
  Identifier ident;
  ident.value = "v2";
  LetStatement let;
  let.name.value = "v1";
  let.expr = ident;

  std::cout << let.String() << "\n";
  std::vector<AstNode> nodes;
  nodes.push_back(let);

  std::cout << let.String() << "\n";
  std::cout << nodes.front().String() << "\n";
}