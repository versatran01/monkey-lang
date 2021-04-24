#pragma once

#include <string>
#include <vector>

#include "monkey/te.hpp"

namespace monkey {

namespace te = boost::te;

struct NodeInterface {
  std::string TokenLiteral() const noexcept {
    return te::call<std::string>([](const auto& self) { self.TokenLiteral(); },
                                 *this);
  }
};

struct Statement {};

struct Expression {};

struct Program {
  std::string TokenLiteral();

  std::vector<Statement> statements;
};

using Node = te::poly<NodeInterface>;

}  // namespace monkey
