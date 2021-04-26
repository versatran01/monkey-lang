#pragma once

#include <string>
#include <vector>

#include "monkey/te.hpp"
#include "monkey/token.h"

namespace monkey {

enum class NodeType { kBase, kProgram, kStatement, kExpression };

// Interface of Node
struct NodeInterface {
  std::string TokenLiteral() const {
    return boost::te::call<std::string>(
        [](const auto &self) { return self.TokenLiteral(); }, *this);
  }

  std::string String() const {
    return boost::te::call<std::string>(
        [](const auto &self) { return self.String(); }, *this);
  }

  NodeType Type() const {
    return boost::te::call<NodeType>(
        [](const auto &self) { return self.Type(); }, *this);
  }

  bool Ok() const {
    return boost::te::call<bool>([](const auto &self) { return self.Ok(); },
                                 *this);
  }
};

using Node = boost::te::poly<NodeInterface>;

struct NodeBase {
  explicit NodeBase(NodeType type = NodeType::kBase) : type{type} {}
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

  std::vector<Node> statements;
};

struct Statement : public NodeBase {
  explicit Statement(NodeType type = NodeType::kStatement) : NodeBase{type} {}
};

struct Expression : public NodeBase {
  explicit Expression(NodeType type = NodeType::kExpression) : NodeBase{type} {}
};

struct Identifier final : public Expression {
  std::string value;
};

struct LetStatement final : public Statement {
  std::string StringImpl() const override;

  Identifier name;
  Expression value;
};

struct ReturnStatement final : public Statement {
  std::string StringImpl() const override;

  Expression value;
};

}  // namespace monkey
