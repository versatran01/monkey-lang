#include "monkey/ast.h"

#include <fmt/core.h>

namespace monkey {

std::string Program::TokenLiteralImpl() const {
  return statements.empty() ? "" : statements.front().TokenLiteral();
}

std::string Program::StringImpl() const {
  std::string res;
  for (const auto& stmt : statements) {
    res += stmt.String() + "\n";
  }
  return res;
}

std::string LetStatement::StringImpl() const {
  return fmt::format("{} {} = {};", TokenLiteral(), name.String(),
                     expr.String());
}

std::string ReturnStatement::StringImpl() const {
  return fmt::format("{} {};", TokenLiteral(), expr.String());
}

std::string PrefixExpression::StringImpl() const {
  return fmt::format("({}{})", op, rhs.String());
}

std::string InfixExpression::StringImpl() const {
  return fmt::format("({} {} {})", lhs.String(), op, rhs.String());
}

}  // namespace monkey
