#pragma once

#include "monkey/token.h"

namespace monkey {

class Lexer {
 public:
  Lexer() = default;
  explicit Lexer(std::string input);

  // Get the next token
  Token NextToken();

 private:
  std::string input_;
  int position_;       // current position in input (points to current char)
  int read_position_;  // current reading position in input (after current char)
  char ch;             // current char under examination
};

}  // namespace monkey