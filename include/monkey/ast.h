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
  kIdentExpr,
  kIntLiteral,
  kPrefixExpr,
  kInfixExpr,
  // Statement
  kExprStmt,
  kLetStmt,
  kReturnStmt
};

// std::ostream &operator<<(std::ostream &os, NodeType node_type);

/// Interface of Node
struct NodeInterface {
  auto TokenLiteral() const {
    return boost::te::call<std::string>(
        [](const auto &self) { return self.TokenLiteral(); }, *this);
  }

  auto String() const {
    return boost::te::call<std::string>(
        [](const auto &self) { return self.String(); }, *this);
  }

  auto Type() const {
    return boost::te::call<NodeType>(
        [](const auto &self) { return self.Type(); }, *this);
  }

  auto Ok() const {
    return boost::te::call<bool>([](const auto &self) { return self.Ok(); },
                                 *this);
  }
};

// fwd
struct ExpressionBase;

/// Interface of expression, extends NodeInterface and also returns a ptr to the
/// underlying node that can be used to recover its original type
struct ExprInterface : public NodeInterface {
  ExprInterface() { boost::te::extends<NodeInterface>(*this); }

  auto *Ptr() const {
    return boost::te::call<ExpressionBase *>(
        [](const auto &self) { return self.Ptr(); }, *this);
  }
};

using Expression = boost::te::poly<ExprInterface>;

struct StmtInterface : public NodeInterface {
  StmtInterface() { boost::te::extends<NodeInterface>(*this); }

  auto Expr() const {
    return boost::te::call<Expression>(
        [](const auto &self) { return self.expr; }, *this);
  }
};

using Statement = boost::te::poly<StmtInterface>;

/// Implementation of Node
struct NodeBase {
  NodeBase() = default;
  explicit NodeBase(NodeType type) : type{type} {}
  virtual ~NodeBase() noexcept = default;

  std::string TokenLiteral() const { return TokenLiteralImpl(); }
  std::string String() const { return StringImpl(); }
  NodeType Type() const { return type; }
  bool Ok() const { return type != NodeType::kInvalid; }

  virtual std::string TokenLiteralImpl() const { return token.literal; }
  virtual std::string StringImpl() const { return token.literal; }

  NodeType type{NodeType::kInvalid};
  Token token;
};

struct Program final : public NodeBase {
  Program() : NodeBase{NodeType::kProgram} {}
  std::string TokenLiteralImpl() const override;
  std::string StringImpl() const override;

  void AddStatement(const Statement &stmt) { statements.push_back(stmt); }
  auto NumStatments() const { return statements.size(); }

  std::vector<Statement> statements;
};

struct ExpressionBase : public NodeBase {
  using NodeBase::NodeBase;

  const ExpressionBase *Ptr() const { return this; }
};

struct Identifier final : public ExpressionBase {
  Identifier() : ExpressionBase{NodeType::kIdentExpr} {}

  std::string value;
};

struct IntegerLiteral final : public ExpressionBase {
  IntegerLiteral() : ExpressionBase{NodeType::kIntLiteral} {}

  int64_t value{};  // use 64 bits
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

struct StatementBase : public NodeBase {
  using NodeBase::NodeBase;

  Expression expr{ExpressionBase{}};
};

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

}  // namespace monkey
