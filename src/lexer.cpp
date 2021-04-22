#include "monkey/lexer.h"

namespace monkey {

Lexer::Lexer(std::string input) : input_(std::move(input)) {}

Token Lexer::NextToken() {
  // TODO: not implemented
  return {};
}
}  // namespace monkey