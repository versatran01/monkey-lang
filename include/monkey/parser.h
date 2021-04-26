#pragma once

#include <absl/container/flat_hash_map.h>

#include <functional>

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

  using PrefixParseFn = std::function<Expression()>;
  using InfixParseFn = std::function<Expression(Expression)>;

  void RegisterInfix(TokenType type, InfixParseFn fn) {
    infix_parse_fn_[type] = std::move(fn);
  }
  void RegisterPrefix(TokenType type, PrefixParseFn fn) {
    prefix_parse_fn_[type] = std::move(fn);
  }

  Lexer lexer_;
  Token curr_token_;
  Token peek_token_;
  std::vector<std::string> errors_;
  absl::flat_hash_map<TokenType, InfixParseFn> infix_parse_fn_;
  absl::flat_hash_map<TokenType, PrefixParseFn> prefix_parse_fn_;
};

}  // namespace monkey
