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
  kStrLiteral,
  kArrayLiteral,
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

/// Erase Actual Node
class AstNode {
 public:
  AstNode() = default;

  template <typename T>
  AstNode(T x) : self_(std::make_shared<T>(std::move(x))) {}

  bool Ok() const { return self_ != nullptr && self_->Ok(); }
  NodeType Type() const noexcept { return self_->Type(); }

  std::string String() const { return self_->String(); }
  std::string TokenLiteral() const { return self_->TokenLiteral(); }
  const NodeBase* Ptr() const noexcept { return self_.get(); }

  template <typename T>
  auto PtrCast() const noexcept {
    return dynamic_cast<const T*>(self_.get());
  }

 protected:
  std::shared_ptr<const NodeBase> self_{nullptr};
};

using StmtNode = AstNode;
using ExprNode = AstNode;

struct Program final : public NodeBase {
  Program() : NodeBase{NodeType::kProgram} {}
  std::string TokenLiteral() const override;
  std::string String() const override;

  auto AddStatement(const StmtNode& stmt) { return statements.push_back(stmt); }
  auto NumStatements() const noexcept { return statements.size(); }

  std::vector<StmtNode> statements;
};

/// Expressions
struct Identifier final : public NodeBase {
  Identifier() : NodeBase{NodeType::kIdentifier} {}

  std::string value;
};

struct IntLiteral final : public NodeBase {
  IntLiteral() : NodeBase{NodeType::kIntLiteral} {}

  int64_t value{};  // use 64 bits
};

struct BoolLiteral final : public NodeBase {
  BoolLiteral() : NodeBase{NodeType::kBoolLiteral} {}

  bool value{};
};

struct StrLiteral final : public NodeBase {
  StrLiteral() : NodeBase{NodeType::kStrLiteral} {}

  std::string value{};
};

struct ArrayLiteral final : public NodeBase {
  ArrayLiteral() : NodeBase{NodeType::kArrayLiteral} {}
  std::string String() const override;

  std::vector<ExprNode> elements;
};

struct PrefixExpr final : public NodeBase {
  PrefixExpr() : NodeBase{NodeType::kPrefixExpr} {}
  std::string String() const override;

  std::string op;
  ExprNode rhs;
};

struct InfixExpr final : public NodeBase {
  InfixExpr() : NodeBase{NodeType::kInfixExpr} {}
  std::string String() const override;

  ExprNode lhs;
  std::string op;
  ExprNode rhs;
};

/// Statements
struct LetStmt final : public NodeBase {
  LetStmt() : NodeBase{NodeType::kLetStmt} {}
  std::string String() const override;

  Identifier name;
  ExprNode expr;
};

struct ReturnStmt final : public NodeBase {
  ReturnStmt() : NodeBase{NodeType::kReturnStmt} {}
  std::string String() const override;

  ExprNode expr;
};

struct ExprStmt final : public NodeBase {
  ExprStmt() : NodeBase{NodeType::kExprStmt} {}
  std::string TokenLiteral() const override { return expr.TokenLiteral(); }
  std::string String() const override { return expr.String(); }

  ExprNode expr;
};

struct BlockStmt final : public NodeBase {
  BlockStmt() : NodeBase{NodeType::kBlockStmt} {}
  std::string String() const override;
  auto size() const noexcept { return statements.size(); }
  bool empty() const noexcept { return statements.empty(); }

  std::vector<StmtNode> statements;
};

struct IfExpr final : public NodeBase {
  IfExpr() : NodeBase{NodeType::kIfExpr} {}
  std::string String() const override;

  ExprNode cond;
  BlockStmt true_block;
  BlockStmt false_block;
};

struct FuncLiteral final : public NodeBase {
  FuncLiteral() : NodeBase{NodeType::kFnLiteral} {}
  std::string String() const override;
  auto NumParams() const noexcept { return params.size(); }

  std::vector<Identifier> params;
  BlockStmt body;
};

struct CallExpr final : public NodeBase {
  CallExpr() : NodeBase{NodeType::kCallExpr} {}
  std::string String() const override;
  auto NumArgs() const noexcept { return args.size(); }

  ExprNode func;
  std::vector<ExprNode> args;
};

// Helper function to get the expression in ExprStmt
const ExprNode& GetExpr(const StmtNode& node);

}  // namespace monkey
