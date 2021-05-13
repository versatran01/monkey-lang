#include "monkey/parser.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_join.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

namespace {

absl::flat_hash_map<TokenType, Precedence> gTokenPrecedence = {
    {TokenType::kEq, Precedence::kEquality},
    {TokenType::kNe, Precedence::kEquality},
    {TokenType::kLt, Precedence::kInequality},
    {TokenType::kLe, Precedence::kInequality},
    {TokenType::kGt, Precedence::kInequality},
    {TokenType::kGe, Precedence::kInequality},
    {TokenType::kPlus, Precedence::kSum},
    {TokenType::kMinus, Precedence::kSum},
    {TokenType::kSlash, Precedence::kProduct},
    {TokenType::kAsterisk, Precedence::kProduct},
    {TokenType::kLParen, Precedence::kCall},
    {TokenType::kLBracket, Precedence::kIndex},
};

Precedence TokenPrecedence(TokenType type) {
  const auto it = gTokenPrecedence.find(type);
  if (it != gTokenPrecedence.end()) {
    return it->second;
  }
  return Precedence::kLowest;
}

}  // namespace

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {
  // Read two tokens, so curToken and peekToken are both set
  NextToken();
  NextToken();
  RegisterParseFns();
}

void Parser::RegisterParseFns() {
  RegisterPrefix(TokenType::kIf, [this]() { return ParseIfExpr(); });
  RegisterPrefix(TokenType::kStr, [this]() { return ParseStrLiteral(); });
  RegisterPrefix(TokenType::kInt, [this]() { return ParseIntLiteral(); });
  RegisterPrefix(TokenType::kFunc, [this]() { return ParseFuncLiteral(); });
  RegisterPrefix(TokenType::kIdent, [this]() { return ParseIdentifier(); });
  RegisterPrefix(TokenType::kTrue, [this]() { return ParseBoolLiteral(); });
  RegisterPrefix(TokenType::kFalse, [this]() { return ParseBoolLiteral(); });
  RegisterPrefix(TokenType::kLParen, [this]() { return ParseGroupedExpr(); });
  RegisterPrefix(TokenType::kLBrace, [this]() { return ParseHashLiteral(); });
  RegisterPrefix(TokenType::kLBracket,
                 [this]() { return ParseArrayLiteral(); });

  auto parse_prefix = [this]() { return ParsePrefixExpr(); };
  RegisterPrefix(TokenType::kBang, parse_prefix);
  RegisterPrefix(TokenType::kMinus, parse_prefix);

  auto parse_infix = [this](const auto& expr) { return ParseInfixExpr(expr); };
  RegisterInfix(TokenType::kPlus, parse_infix);
  RegisterInfix(TokenType::kMinus, parse_infix);
  RegisterInfix(TokenType::kSlash, parse_infix);
  RegisterInfix(TokenType::kAsterisk, parse_infix);
  RegisterInfix(TokenType::kEq, parse_infix);
  RegisterInfix(TokenType::kNe, parse_infix);
  RegisterInfix(TokenType::kLt, parse_infix);
  RegisterInfix(TokenType::kGt, parse_infix);
  RegisterInfix(TokenType::kLe, parse_infix);
  RegisterInfix(TokenType::kGe, parse_infix);
  RegisterInfix(TokenType::kLParen,
                [this](const auto& expr) { return PasrseCallExpr(expr); });
  RegisterInfix(TokenType::kLBracket,
                [this](const auto& expr) { return ParseIndexExpr(expr); });
}

void Parser::NextToken() {
  curr_token_ = peek_token_;
  peek_token_ = lexer_.NextToken();
}

Program Parser::ParseProgram() {
  Program program;
  while (curr_token_.type != TokenType::kEof) {
    const StmtNode stmt = ParseStatement();
    if (stmt.Ok()) {
      program.statements.push_back(stmt);
    }
    NextToken();
  }
  return program;
}

std::string Parser::ErrorMsg() const { return absl::StrJoin(errors_, "\n"); }

StmtNode Parser::ParseStatement() {
  switch (curr_token_.type) {
    case TokenType::kLet:
      return ParseLetStmt();
    case TokenType::kReturn:
      return ParseReturnStmt();
    default:
      return PasreExprStmt();
  }
}

StmtNode Parser::ParseLetStmt() {
  LetStmt let_stmt;
  let_stmt.token = curr_token_;

  if (!ExpectPeek(TokenType::kIdent)) {
    LOG(INFO) << "[ParseLetStatement] Next token is not Ident";
    return {};
  }

  let_stmt.name.token = curr_token_;
  let_stmt.name.value = curr_token_.literal;

  if (!ExpectPeek(TokenType::kAssign)) {
    LOG(INFO) << "[ParseLetStatement] Next token is not Assing";
    return {};
  }

  NextToken();
  let_stmt.expr = ParseExpression(Precedence::kLowest);

  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return let_stmt;
}

StmtNode Parser::ParseReturnStmt() {
  ReturnStmt ret_stmt;
  ret_stmt.token = curr_token_;

  NextToken();
  ret_stmt.expr = ParseExpression(Precedence::kLowest);

  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return ret_stmt;
}

StmtNode Parser::PasreExprStmt() {
  ExprStmt expr_stmt;
  expr_stmt.token = curr_token_;
  expr_stmt.expr = ParseExpression(Precedence::kLowest);

  if (IsPeekToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return expr_stmt;
}

BlockStmt Parser::ParseBlockStmt() {
  BlockStmt block_stmt;
  block_stmt.token = curr_token_;

  NextToken();

  while (!IsCurrToken(TokenType::kRBrace) && !IsCurrToken(TokenType::kEof)) {
    auto stmt = ParseStatement();
    if (stmt.Ok()) {
      block_stmt.statements.push_back(std::move(stmt));
    }
    NextToken();
  }

  return block_stmt;
}

ExprNode Parser::ParseExpression(Precedence precedence) {
  const auto prefix_it = prefix_parse_fn_.find(curr_token_.type);
  if (prefix_it == prefix_parse_fn_.end()) {
    const std::string msg =
        fmt::format("no prefix parse function for {}", curr_token_.type);
    LOG(WARNING) << msg;
    errors_.push_back(msg);
    return {};
  }

  auto lhs = prefix_it->second();

  while (!IsPeekToken(TokenType::kSemicolon) && precedence < PeekPrecedence()) {
    const auto infix_it = infix_parse_fn_.find(peek_token_.type);
    if (infix_it == infix_parse_fn_.end()) {
      return lhs;
    }
    NextToken();
    lhs = infix_it->second(lhs);
  }

  return lhs;
}

ExprNode Parser::ParseIdentifier() {
  Identifier ident;
  ident.token = curr_token_;
  ident.value = curr_token_.literal;
  return ident;
}

ExprNode Parser::ParseStrLiteral() {
  StrLiteral str;
  str.token = curr_token_;
  str.value = curr_token_.literal;
  return str;
}

ExprNode Parser::ParseIntLiteral() {
  IntLiteral intl;
  intl.token = curr_token_;

  bool ok = absl::SimpleAtoi(intl.token.literal, &intl.value);
  if (!ok) {
    const auto msg =
        fmt::format("could not parse {} as integer", curr_token_.literal);
    LOG(WARNING) << msg;
    errors_.push_back(msg);
    return {};
  }

  return intl;
}

ExprNode Parser::ParseBoolLiteral() {
  BoolLiteral booll;
  booll.token = curr_token_;
  booll.value = IsCurrToken(TokenType::kTrue);
  return booll;
}

ExprNode Parser::ParseFuncLiteral() {
  FuncLiteral func;
  func.token = curr_token_;

  if (!ExpectPeek(TokenType::kLParen)) {
    return {};
  }

  func.params = ParseFuncParams();

  if (!ExpectPeek(TokenType::kLBrace)) {
    return {};
  }

  func.body = ParseBlockStmt();
  return func;
}

ExprNode Parser::ParseHashLiteral() {
  DictLiteral hash;
  hash.token = curr_token_;

  while (!IsPeekToken(TokenType::kRBrace)) {
    NextToken();
    auto key = ParseExpression(Precedence::kLowest);

    if (!ExpectPeek(TokenType::kColon)) {
      return {};
    }

    NextToken();
    auto val = ParseExpression(Precedence::kLowest);

    hash.pairs.push_back({key, val});

    if (!IsPeekToken(TokenType::kRBrace) && !ExpectPeek(TokenType::kComma)) {
      return {};
    }
  }

  if (!ExpectPeek(TokenType::kRBrace)) {
    return {};
  }

  return hash;
}

ExprNode Parser::ParseArrayLiteral() {
  ArrayLiteral arr;
  arr.token = curr_token_;
  arr.elements = ParseExprList(TokenType::kRBracket);
  return arr;
}

ExprNode Parser::ParseInfixExpr(const ExprNode& lhs) {
  InfixExpr infx;
  infx.token = curr_token_;
  infx.op = curr_token_.literal;
  infx.lhs = lhs;

  const auto precedence = CurrPrecedence();
  NextToken();
  infx.rhs = ParseExpression(precedence);
  return infx;
}

ExprNode Parser::ParseIndexExpr(const ExprNode& expr) {
  IndexExpr index;
  index.token = curr_token_;
  index.lhs = expr;

  NextToken();
  index.index = ParseExpression(Precedence::kLowest);

  if (!ExpectPeek(TokenType::kRBracket)) {
    return {};
  }

  return index;
}

std::vector<ExprNode> Parser::ParseExprList(TokenType end_type) {
  std::vector<ExprNode> exprs;

  if (IsPeekToken(end_type)) {
    NextToken();
    return exprs;
  }

  NextToken();
  exprs.push_back(ParseExpression(Precedence::kLowest));

  while (IsPeekToken(TokenType::kComma)) {
    NextToken();
    NextToken();
    exprs.push_back(ParseExpression(Precedence::kLowest));
  }

  if (!ExpectPeek(end_type)) {
    return {};
  }

  return exprs;
}

ExprNode Parser::ParseGroupedExpr() {
  NextToken();
  auto expr = ParseExpression(Precedence::kLowest);
  if (!ExpectPeek(TokenType::kRParen)) {
    return {};
  }
  return expr;
}

ExprNode Parser::ParseIfExpr() {
  IfExpr if_expr;
  if_expr.token = curr_token_;

  if (!ExpectPeek(TokenType::kLParen)) {
    return {};
  }

  NextToken();
  if_expr.cond = ParseExpression(Precedence::kLowest);

  if (!ExpectPeek(TokenType::kRParen)) {
    return {};
  }
  if (!ExpectPeek(TokenType::kLBrace)) {
    return {};
  }
  if_expr.true_block = ParseBlockStmt();

  // else
  if (IsPeekToken(TokenType::kElse)) {
    NextToken();
    if (!ExpectPeek(TokenType::kLBrace)) {
      return {};
    }
    if_expr.false_block = ParseBlockStmt();
  }

  return if_expr;
}

ExprNode Parser::PasrseCallExpr(const ExprNode& expr) {
  CallExpr call;
  call.token = curr_token_;
  call.func = expr;
  call.args = ParseExprList(TokenType::kRParen);
  return call;
}

std::vector<Identifier> Parser::ParseFuncParams() {
  std::vector<Identifier> params;

  if (IsPeekToken(TokenType::kRParen)) {
    NextToken();
    return params;
  }

  NextToken();

  Identifier param;
  param.token = curr_token_;
  param.value = curr_token_.literal;
  params.push_back(param);

  while (IsPeekToken(TokenType::kComma)) {
    NextToken();
    NextToken();

    param.token = curr_token_;
    param.value = curr_token_.literal;
    params.push_back(param);
  }

  if (!ExpectPeek(TokenType::kRParen)) {
    return {};
  }

  return params;
}

ExprNode Parser::ParsePrefixExpr() {
  PrefixExpr prefix;
  prefix.token = curr_token_;
  prefix.op = curr_token_.literal;

  NextToken();
  prefix.rhs = ParseExpression(Precedence::kPrefix);

  return prefix;
}

bool Parser::ExpectPeek(TokenType type) {
  if (IsPeekToken(type)) {
    NextToken();
    return true;
  }

  PeekError(type);
  return false;
}

void Parser::PeekError(TokenType type) {
  errors_.push_back(fmt::format(
      "Expected next token to be {}, got {} instead", type, peek_token_.type));
}

Precedence Parser::CurrPrecedence() const {
  return TokenPrecedence(curr_token_.type);
}

Precedence Parser::PeekPrecedence() const {
  return TokenPrecedence(peek_token_.type);
}

}  // namespace monkey
