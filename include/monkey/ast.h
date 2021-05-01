#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "monkey/te.hpp"
#include "monkey/token.h"

namespace monkey {

enum class NodeType {
  kBase,
  kProgram,
  // Expression
  kIdentExpr,
  // Statment
  kExprStmt,
  kLetStmt,
  kReturnStmt
};

// std::ostream &operator<<(std::ostream &os, NodeType node_type);

// Interface of Node
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

using ExprNode = boost::te::poly<NodeInterface>;

struct StmtInterface : public NodeInterface {
  StmtInterface() { boost::te::extends<NodeInterface>(*this); }

  auto Expr() const {
    return boost::te::call<ExprNode>([](const auto &self) { return self.expr; },
                                     *this);
  }
};

using StmtNode = boost::te::poly<StmtInterface>;

// Types of Node
struct NodeBase {
  NodeBase() = default;
  explicit NodeBase(NodeType type) : type{type} {}
  virtual ~NodeBase() noexcept = default;

  std::string TokenLiteral() const { return TokenLiteralImpl(); }
  std::string String() const { return StringImpl(); }
  NodeType Type() const { return type; }
  bool Ok() const { return type != NodeType::kBase; }

  virtual std::string TokenLiteralImpl() const { return token.literal; }
  virtual std::string StringImpl() const { return token.literal; }

  NodeType type{NodeType::kBase};
  Token token;
};

struct Program final : public NodeBase {
  Program() : NodeBase{NodeType::kProgram} {}
  std::string TokenLiteralImpl() const override;
  std::string StringImpl() const override;

  void AddStatement(const StmtNode &stmt) { statements.push_back(stmt); }
  auto NumStatments() const { return statements.size(); }

  std::vector<StmtNode> statements;
};

struct Expression : public NodeBase {
  using NodeBase::NodeBase;
};

struct Identifier final : public Expression {
  Identifier() : Expression{NodeType::kIdentExpr} {}
  std::string StringImpl() const override { return value; }

  std::string value;
};

struct Statement : public NodeBase {
  using NodeBase::NodeBase;

  ExprNode expr{Expression{}};
};

struct LetStatement final : public Statement {
  LetStatement() : Statement{NodeType::kLetStmt} {}
  std::string StringImpl() const override;

  Identifier name;
};

struct ReturnStatement final : public Statement {
  ReturnStatement() : Statement{NodeType::kReturnStmt} {}
  std::string StringImpl() const override;
};

struct ExpressionStatement final : public Statement {
  ExpressionStatement() : Statement{NodeType::kExprStmt} {}
  std::string TokenLiteralImpl() const override { return expr.TokenLiteral(); }
};

}  // namespace monkey
