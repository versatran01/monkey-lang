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

 private:
  void NextToken();
  Node ParseStatement();
  Node ParseLetStatement();

  bool IsCurrToken(TokenType type) const { return curr_token_.type == type; }
  bool IsPeekToken(TokenType type) const { return peek_token_.type == type; }
  bool ExpectPeek(TokenType type);

  Lexer lexer_;
  Token curr_token_;
  Token peek_token_;
};

}  // namespace monkey
