#include "monkey/ast.h"

namespace monkey {

std::string Program::TokenLiteralImpl() {
  if (statements.empty()) {
    return {};
  }
  return statements.front().TokenLiteral();
}

}  // namespace monkey
