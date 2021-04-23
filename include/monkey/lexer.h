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
  void ReadChar();
  std::string ReadIdentifier();

  std::string input_;
  int position_{0};       // current position in input (points to current char)
  int read_position_{0};  // current read position in input (after current char)
  char ch_;               // current char under examination
};

}  // namespace monkey
