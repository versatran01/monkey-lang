#include "monkey/ast.h"

#include <fmt/core.h>

namespace monkey {

std::string Program::TokenLiteralImpl() const {
  if (statements.empty()) {
    return {};
  }
  return statements.front().TokenLiteral();
}

std::string Program::StringImpl() const {
  std::string res;
  for (const auto& stmt : statements) {
    res += stmt.String() + "\n";
  }
  return res;
}

std::string LetStatement::StringImpl() const {
  // let name = expr;
  return fmt::format("{} {} = {};", TokenLiteral(), name.String(),
                     value.String());
}

std::string ReturnStatement::StringImpl() const {
  return fmt::format("{} {};", TokenLiteral(), value.String());
}

}  // namespace monkey
