#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "monkey/token.h"

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
  explicit NodeBase(NodeType type) : type_{type} {}
  virtual ~NodeBase() noexcept = default;

  NodeType Type() const noexcept { return type_; }
  bool Ok() const noexcept { return type_ != NodeType::kInvalid; }

  virtual std::string String() const { return token.literal; };
  virtual std::string TokenLiteral() const { return token.literal; }

 private:
  NodeType type_{NodeType::kInvalid};

 public:
  Token token;
};

class AstNode {
 public:
  AstNode() = default;

  template <typename T>
  AstNode(T x) : self_(std::make_shared<Model<T>>(std::move(x))) {}

  std::string String() const { return self_->String(); }
  std::string TokenLiteral() const { return self_->TokenLiteral(); }
  bool Ok() const { return self_ != nullptr; }

  template <typename T>
  auto Ptr() const {
    return dynamic_cast<T*>(self_->Ptr());
  }

 private:
  struct Concept {
    virtual ~Concept() noexcept = default;
    virtual std::string String() const = 0;
    virtual std::string TokenLiteral() const = 0;
  };

  template <typename T>
  struct Model final : public Concept {
    explicit Model(T x) : data_(std::move(x)) {}
    std::string String() const override { return data_.String(); }
    std::string TokenLiteral() const override { return data_.TokenLiteral(); }
    const NodeBase* Ptr() const noexcept { return &data_; }

    T data_;
  };

  std::shared_ptr<const Concept> self_{nullptr};
};

using ExprNode = AstNode;
using StmtNode = AstNode;

struct Program final : public NodeBase {
  Program() : NodeBase{NodeType::kProgram} {}
  std::string TokenLiteral() const override;
  std::string String() const override;

  auto AddStatement(const StmtNode& stmt) { return statements.push_back(stmt); }
  auto NumStatements() const noexcept { return statements.size(); }

  std::vector<StmtNode> statements;
};

/// Base statement
struct StmtBase : public NodeBase {
  using NodeBase::NodeBase;

  ExprNode expr;
};

/// Expressions
struct Identifier final : public NodeBase {
  Identifier(std::string value = {})
      : NodeBase{NodeType::kIdentifier}, value{std::move(value)} {}

  std::string value;
};

struct IntLiteral final : public NodeBase {
  IntLiteral(int64_t value = {})
      : NodeBase{NodeType::kIntLiteral}, value{value} {}

  int64_t value{};  // use 64 bits
};

struct BoolLiteral final : public NodeBase {
  BoolLiteral(bool value = {})
      : NodeBase{NodeType::kBoolLiteral}, value{value} {}

  bool value{};
};

struct PrefixExpression final : public NodeBase {
  PrefixExpression() : NodeBase{NodeType::kPrefixExpr} {}
  std::string String() const override;

  std::string op;
  ExprNode rhs;
};

struct InfixExpression final : public NodeBase {
  InfixExpression() : NodeBase{NodeType::kInfixExpr} {}
  std::string String() const override;

  ExprNode lhs;
  std::string op;
  ExprNode rhs;
};

/// Statements
struct LetStatement final : public StmtBase {
  LetStatement() : StmtBase{NodeType::kLetStmt} {}
  std::string String() const override;

  Identifier name;
};

struct ReturnStatement final : public StmtBase {
  ReturnStatement() : StmtBase{NodeType::kReturnStmt} {}
  std::string String() const override;
};

struct ExpressionStatement final : public StmtBase {
  ExpressionStatement() : StmtBase{NodeType::kExprStmt} {}
  std::string TokenLiteral() const override { return expr.TokenLiteral(); }
  std::string String() const override { return expr.String(); }
};

struct BlockStatement final : public StmtBase {
  BlockStatement() : StmtBase{NodeType::kBlockStmt} {}
  std::string String() const override;
  auto size() const noexcept { return statements.size(); }
  bool empty() const noexcept { return statements.empty(); }

  std::vector<StmtNode> statements;
};

struct IfExpression final : public NodeBase {
  IfExpression() : NodeBase{NodeType::kIfExpr} {}
  std::string String() const override;

  ExprNode cond;
  BlockStatement true_block;
  BlockStatement false_block;
};

struct FunctionLiteral final : public NodeBase {
  FunctionLiteral() : NodeBase{NodeType::kFnLiteral} {}
  std::string String() const override;
  auto NumParams() const noexcept { return params.size(); }

  std::vector<Identifier> params;
  BlockStatement body;
};

struct CallExpression final : public NodeBase {
  CallExpression() : NodeBase{NodeType::kCallExpr} {}
  std::string String() const override;
  auto NumArgs() const noexcept { return args.size(); }

  ExprNode func;
  std::vector<ExprNode> args;
};

}  // namespace monkey
