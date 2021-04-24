#pragma once

#include <string>
#include <vector>

#include "monkey/te.hpp"
#include "monkey/token.h"

namespace monkey {

namespace te = boost::te;

enum class NodeType { kBase, kStatement, kExpression };

struct NodeInterface {
  std::string TokenLiteral() const noexcept {
    return te::call<std::string>([](const auto &self) { self.TokenLiteral(); },
                                 *this);
  }
};

using Node = te::poly<NodeInterface>;

struct NodeBase {
  std::string TokenLiteral();
  std::string String();
  std::string Type();

  virtual std::string TokenLiteralImpl();
  virtual std::string StringImpl();
  virtual std::string TypeImpl();

  NodeType type{NodeType::kBase};
};

struct Statement : public NodeBase {};

struct Expression : public NodeBase {};

struct Program final : public NodeBase {
  std::string TokenLiteralImpl() override;
  std::string StringImpl() override;
  std::string TypeImpl() override;

  std::vector<Node> statements;
};

struct Identifier {
  Token token;
  std::string value;
};

struct LetStatement : public Statement {
  Token token;
  // Identifier name;
  Expression value;
};

}  // namespace monkey
