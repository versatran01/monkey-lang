#pragma once

#include <absl/container/flat_hash_map.h>

#include <functional>

#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/token.h"

namespace monkey {

enum class Precedence {
  kLowest,
  kEquality,
  kInequality,
  kSum,
  kProduct,
  kPrefix,
  kCall,
  kIndex
};

class Parser {
 public:
  explicit Parser(Lexer lexer);
  explicit Parser(std::string input) : Parser{Lexer{std::move(input)}} {}

  Program ParseProgram();
  std::string ErrorMsg() const;
  bool Ok() const noexcept { return errors_.empty(); }

 private:
  // Parsing functions
  StmtNode ParseStatement();
  StmtNode ParseLetStmt();
  StmtNode PasreExprStmt();
  StmtNode ParseReturnStmt();
  BlockStmt ParseBlockStmt();

  ExprNode ParseExpression(Precedence precedence);
  ExprNode ParseIdentifier();
  ExprNode ParseStrLiteral();
  ExprNode ParseIntLiteral();
  ExprNode ParseBoolLiteral();
  ExprNode ParseFuncLiteral();
  ExprNode ParseHashLiteral();
  ExprNode ParseArrayLiteral();

  ExprNode ParseIfExpr();
  ExprNode ParsePrefixExpr();
  ExprNode ParseGroupedExpr();
  ExprNode PasrseCallExpr(const ExprNode& expr);
  ExprNode ParseInfixExpr(const ExprNode& expr);
  ExprNode ParseIndexExpr(const ExprNode& expr);

  std::vector<Identifier> ParseFuncParams();
  std::vector<ExprNode> ParseExprList(TokenType end_type);

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

  Precedence CurrPrecedence() const;
  Precedence PeekPrecedence() const;

  using PrefixParseFn = std::function<ExprNode()>;
  using InfixParseFn = std::function<ExprNode(ExprNode)>;

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
