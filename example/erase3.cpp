
#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <vector>

enum class NodeType {
  kInvalid,
  kProgram,
  // Expression
  kIdentifier,
  // Statement
  kLetStmt,
};

/// Base Node
struct NodeBase {
  explicit NodeBase(NodeType type) : type{type} {}
  virtual ~NodeBase() noexcept = default;

  NodeType Type() const noexcept { return type; }
  virtual std::string String() const { return ""; }
  bool Ok() const noexcept { return type != NodeType::kInvalid; }

  NodeType type{NodeType::kInvalid};
};

class AstNode {
 public:
  AstNode() = default;

  template <typename T>
  AstNode(T x) : self_(std::make_shared<T>(std::move(x))) {}

  std::string String() const { return self_->String(); }
  NodeType Type() const noexcept { return self_->Type(); }
  bool Ok() const noexcept { return self_ != nullptr && self_->Ok(); }
  const NodeBase* Ptr() const noexcept { return self_.get(); }
  template <typename T>
  auto PtrCast() const noexcept {
    return dynamic_cast<const T*>(self_.get());
  }

 private:
  std::shared_ptr<const NodeBase> self_{nullptr};
};

/// Expressions
struct Identifier final : public NodeBase {
  Identifier() : NodeBase{NodeType::kIdentifier} {}
  std::string String() const override { return value; }

  std::string value;
};

/// Statements
struct LetStatement final : public NodeBase {
  LetStatement() : NodeBase{NodeType::kLetStmt} {}
  std::string String() const override {
    return "let " + name.String() + " = " + expr.String();
  }

  Identifier name;
  AstNode expr;
};

int main() {
  Identifier ident;
  ident.value = "v2";
  std::cout << ident.String() << "\n";

  LetStatement let;
  let.name.value = "v1";
  let.expr = ident;
  std::cout << let.expr.String() << "\n";
  std::cout << let.String() << "\n";

  std::vector<AstNode> nodes;
  nodes.push_back(let);

  std::cout << ident.String() << "\n";
  std::cout << let.String() << "\n";
  std::cout << nodes.front().String() << "\n";

  const auto* ptr = let.expr.PtrCast<Identifier>();
  if (ptr == nullptr) {
    std::cout << "cast failed\n";
  } else {
    std::cout << "cast good\n";
  }
}
