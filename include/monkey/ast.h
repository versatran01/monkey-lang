#pragma once

#include <string>
#include <vector>

#include "monkey/te.hpp"
#include "monkey/token.h"

namespace monkey {

enum class NodeType { kBase, kProgram, kStatement, kExpression };

// Interface of Node
struct NodeInterface {
  std::string TokenLiteral() const noexcept {
    return boost::te::call<std::string>(
        [](const auto &self) { self.TokenLiteral(); }, *this);
  }

  std::string String() const noexcept {
    return boost::te::call<std::string>([](const auto &self) { self.String(); },
                                        *this);
  }

  NodeType Type() const noexcept {
    return boost::te::call<NodeType>([](const auto &self) { self.Type(); },
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

  virtual std::string TokenLiteralImpl() const { return token.literal; }
  virtual std::string StringImpl() const { return {}; }

  NodeType type{NodeType::kBase};
  Token token;
};

struct Program final : public NodeBase {
  Program() : NodeBase(NodeType::kProgram) {}
  std::string TokenLiteralImpl() const override;

  std::vector<Node> statements;
};

struct Statement : public NodeBase {
  explicit Statement(NodeType type = NodeType::kStatement) : NodeBase{type} {}
};

struct Expression : public NodeBase {};

struct Identifier final : public Expression {
  std::string value;
};

struct LetStatement final : public Statement {
  Identifier name;
  Expression value;
};

}  // namespace monkey
