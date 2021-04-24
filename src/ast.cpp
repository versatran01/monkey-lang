#include "monkey/ast.h"

namespace monkey {

std::string Program::TokenLiteralImpl() const {
  if (statements.empty()) {
    return {};
  }
  return statements.front().TokenLiteral();
}

}  // namespace monkey
