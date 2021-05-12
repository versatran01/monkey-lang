#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <vector>

class AstNode {
 public:
  AstNode() = default;

  template <typename T>
  AstNode(T x) : self_(std::make_shared<Model<T>>(std::move(x))) {}

  std::string String() const { return self_->String(); }

 private:
  struct Concept {
    virtual ~Concept() noexcept = default;
    virtual std::string String() const { return ""; };
  };

  template <typename T>
  struct Model final : public Concept {
    Model(T x) : data_(std::move(x)) {}
    std::string String() const override { return data_.String(); }

    T data_;
  };

  std::shared_ptr<const Concept> self_{nullptr};
};

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
  AstNode expr;
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
