#pragma once

#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/token.h"

namespace monkey {

class Parser {
 public:
  explicit Parser(Lexer lexer);
  explicit Parser(std::string input) : Parser{Lexer{input}} {}

  Program ParseProgram();
  std::vector<std::string> errors() const noexcept { return errors_; }

 private:
  void NextToken();
  Node ParseStatement();
  Node ParseLetStatement();
  Node ParseReturnStatement();

  bool IsCurrToken(TokenType type) const { return curr_token_.type == type; }
  bool IsPeekToken(TokenType type) const { return peek_token_.type == type; }
  bool ExpectPeek(TokenType type);
  void PeekError(TokenType type);

  Lexer lexer_;
  Token curr_token_;
  Token peek_token_;
  std::vector<std::string> errors_;
};

}  // namespace monkey
