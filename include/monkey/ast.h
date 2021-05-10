#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "monkey/te.hpp"
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
  NodeBase() noexcept = default;
  explicit NodeBase(NodeType type, Token token = {}) noexcept
      : type_{type}, token{std::move(token)} {}
  virtual ~NodeBase() noexcept = default;

  NodeType Type() const noexcept { return type_; }
  const NodeBase* Ptr() const noexcept { return this; }
  bool Ok() const noexcept { return type_ != NodeType::kInvalid; }

  std::string String() const noexcept { return StringImpl(); }
  std::string TokenLiteral() const noexcept { return TokenLiteralImpl(); }
  virtual std::string StringImpl() const { return token.literal; }
  virtual std::string TokenLiteralImpl() const { return token.literal; }

 private:
  NodeType type_{NodeType::kInvalid};

 public:
  Token token;
};

/// Interface of Node
struct NodeInterface {
  auto TokenLiteral() const {
    return boost::te::call<std::string>(
        [](const auto& self) { return self.TokenLiteral(); }, *this);
  }

  auto String() const {
    return boost::te::call<std::string>(
        [](const auto& self) { return self.String(); }, *this);
  }

  auto Type() const noexcept {
    return boost::te::call<NodeType>(
        [](const auto& self) { return self.Type(); }, *this);
  }

  auto Ok() const noexcept {
    return boost::te::call<bool>([](const auto& self) { return self.Ok(); },
                                 *this);
  }

  auto* Ptr() const noexcept {
    return boost::te::call<NodeBase*>(
        [](const auto& self) { return self.Ptr(); }, *this);
  }

  template <typename D>
  auto PtrCast() const {
    static_assert(std::is_base_of_v<NodeBase, D>,
                  "D is not derived from NodeBase");
    return static_cast<const D*>(Ptr());
  }
};

using AstNode = boost::te::poly<NodeInterface>;

/// Interface of expression, extends NodeInterface and also returns a ptr to the
/// underlying node that can be used to recover its original type
struct ExprInterface : public NodeInterface {
  ExprInterface() { boost::te::extends<NodeInterface>(*this); }
};

using Expression = boost::te::poly<ExprInterface>;

struct StmtInterface : public NodeInterface {
  StmtInterface() { boost::te::extends<NodeInterface>(*this); }

  auto Expr() const {
    return boost::te::call<Expression>(
        [](const auto& self) { return self.expr; }, *this);
  }
};

using Statement = boost::te::poly<StmtInterface>;

struct Program final : public NodeBase {
  Program() : NodeBase{NodeType::kProgram} {}
  std::string TokenLiteralImpl() const override;
  std::string StringImpl() const override;

  void AddStatement(const Statement& stmt) { statements.push_back(stmt); }
  auto NumStatements() const noexcept { return statements.size(); }

  std::vector<Statement> statements;
};

/// Base expression
struct ExpressionBase : public NodeBase {
  using NodeBase::NodeBase;
};

/// Base statement
struct StatementBase : public NodeBase {
  using NodeBase::NodeBase;

  Expression expr{ExpressionBase{}};
};

/// Expressions
struct Identifier final : public ExpressionBase {
  Identifier() : ExpressionBase{NodeType::kIdentifier} {}

  std::string value;
};

struct IntLiteral final : public ExpressionBase {
  IntLiteral() : ExpressionBase{NodeType::kIntLiteral} {}

  int64_t value{};  // use 64 bits
};

struct BoolLiteral final : public ExpressionBase {
  BoolLiteral() : ExpressionBase{NodeType::kBoolLiteral} {}

  bool value{};
};

struct PrefixExpression final : public ExpressionBase {
  PrefixExpression() : ExpressionBase{NodeType::kPrefixExpr} {}
  std::string StringImpl() const override;

  std::string op;
  Expression rhs{ExpressionBase{}};
};

struct InfixExpression final : public ExpressionBase {
  InfixExpression() : ExpressionBase{NodeType::kInfixExpr} {}
  std::string StringImpl() const override;

  Expression lhs{ExpressionBase{}};
  std::string op;
  Expression rhs{ExpressionBase{}};
};

/// Statements
struct LetStatement final : public StatementBase {
  LetStatement() : StatementBase{NodeType::kLetStmt} {}
  std::string StringImpl() const override;

  Identifier name;
};

struct ReturnStatement final : public StatementBase {
  ReturnStatement() : StatementBase{NodeType::kReturnStmt} {}
  std::string StringImpl() const override;
};

struct ExpressionStatement final : public StatementBase {
  ExpressionStatement() : StatementBase{NodeType::kExprStmt} {}
  std::string TokenLiteralImpl() const override { return expr.TokenLiteral(); }
  std::string StringImpl() const override;
};

struct BlockStatement final : public StatementBase {
  BlockStatement() : StatementBase{NodeType::kBlockStmt} {}
  std::string StringImpl() const override;
  auto size() const noexcept { return statements.size(); }
  bool empty() const noexcept { return statements.empty(); }

  std::vector<Statement> statements;
};

struct IfExpression final : public ExpressionBase {
  IfExpression() : ExpressionBase{NodeType::kIfExpr} {}
  std::string StringImpl() const override;

  Expression cond{ExpressionBase{}};
  BlockStatement true_block;
  BlockStatement false_block;
};

struct FunctionLiteral final : public ExpressionBase {
  FunctionLiteral() : ExpressionBase{NodeType::kFnLiteral} {}
  std::string StringImpl() const override;
  auto NumParams() const noexcept { return params.size(); }

  std::vector<Identifier> params;
  BlockStatement body;
};

struct CallExpression final : public ExpressionBase {
  CallExpression() : ExpressionBase{NodeType::kCallExpr} {}
  std::string StringImpl() const override;
  auto NumArgs() const noexcept { return args.size(); }

  Expression func{ExpressionBase{}};
  std::vector<Expression> args;
};

}  // namespace monkey
