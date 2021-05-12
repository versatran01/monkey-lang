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

  NodeType type{NodeType::kInvalid};
};

class AstNode {
 public:
  AstNode() = default;

  template <typename T>
  AstNode(T x) : self_(std::make_shared<Model<T>>(std::move(x))) {}
  //  AstNode(T x) : self_(std::make_shared<NodeBase>(std::move(x))) {}

  std::string String() const { return self_->String(); }

 private:
  struct Concept {
    virtual ~Concept() noexcept = default;
    virtual std::string String() const = 0;
  };

  template <typename T>
  struct Model final : public Concept {
    explicit Model(T x) : data_(std::move(x)) {}
    std::string String() const override { return data_.String(); }

    T data_;
  };

  std::shared_ptr<const Concept> self_{nullptr};
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
}