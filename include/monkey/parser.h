#pragma once

#include <absl/container/flat_hash_map.h>

#include <functional>

#include "monkey/ast.h"
#include "monkey/lexer.h"  // token

namespace monkey {

enum class Precedence {
  kLowest,
  kEquality,
  kInequality,
  kSum,
  kProduct,
  kPrefix,
  kCall
};

class Parser {
 public:
  explicit Parser(Lexer lexer);
  explicit Parser(const std::string& input) : Parser{Lexer{input}} {}

  Program ParseProgram();
  std::string ErrorMsg() const;
  bool Ok() const noexcept { return errors_.empty(); }

 private:
  // Parsing functions
  Statement ParseStatement();
  Statement ParseLetStatement();
  Statement ParseReturnStatement();
  Statement ParseExpressionStatement();
  BlockStatement ParseBlockStatement();

  Expression ParseExpression(Precedence precedence);
  Expression ParseIdentifier();
  Expression ParseIntegerLiteral();
  Expression ParseBooleanLiteral();
  Expression ParseFunctionLiteral();

  Expression ParseIfExpression();
  Expression ParsePrefixExpression();
  Expression ParseGroupedExpression();
  Expression ParseCallExpression(const Expression& expr);
  Expression ParseInfixExpression(const Expression& expr);

  std::vector<Identifier> ParseFunctionParameters();
  std::vector<Expression> ParseCallArguments();

  // Token functions
  void NextToken();
  bool IsCurrToken(TokenType type) const noexcept {
    return curr_token_.type == type;
  }
  bool IsPeekToken(TokenType type) const noexcept {
    return peek_token_.type == type;
  }
  bool ExpectPeek(TokenType type);
  void PeekError(TokenType type);

  Precedence TokenPrecedence(TokenType type) const;
  Precedence CurrPrecedence() const;
  Precedence PeekPrecedence() const;

  using PrefixParseFn = std::function<Expression()>;
  using InfixParseFn = std::function<Expression(Expression)>;

  void RegisterParseFns();
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
