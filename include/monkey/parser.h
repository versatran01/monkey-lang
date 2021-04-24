#pragma once

#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/token.h"

namespace monkey {

class Parser {
 public:
  explicit Parser(Lexer& lexer);

  Program ParseProgram();

 private:
  void NextToken();

  Lexer& lexer_;
  Token curr_token_;
  Token peek_token_;
};

}  // namespace monkey
