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
  void ReadChar() noexcept;
  void SkipWhitespace();

  char PeekChar() const noexcept;
  std::string ReadNumber();
  std::string ReadIdentifier();
  std::string ReadString();
  Token ReadDualToken(TokenType type1, char next_ch, TokenType type2) noexcept;

  std::string input_;
  std::size_t position_{0};       // current position in input (current char)
  std::size_t read_position_{0};  // current read position  (after current char)
  char ch_{0};                    // current char under examination
};

}  // namespace monkey
