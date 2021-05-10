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
    {TokenType::kLParen, Precedence::kCall}};

}  // namespace

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {
  // Read two tokens, so curToken and peekToken are both set
  NextToken();
  NextToken();
  RegisterParseFns();
}

void Parser::RegisterParseFns() {
  RegisterPrefix(TokenType::kIf, [this]() { return ParseIfExpression(); });
  RegisterPrefix(TokenType::kIdent, [this]() { return ParseIdentifier(); });
  RegisterPrefix(TokenType::kInt, [this]() { return ParseIntegerLiteral(); });
  RegisterPrefix(TokenType::kTrue, [this]() { return ParseBooleanLiteral(); });
  RegisterPrefix(TokenType::kFalse, [this]() { return ParseBooleanLiteral(); });
  RegisterPrefix(TokenType::kFunc, [this]() { return ParseFunctionLiteral(); });
  RegisterPrefix(TokenType::kLParen,
                 [this]() { return ParseGroupedExpression(); });

  auto parse_prefix = [this]() { return ParsePrefixExpression(); };
  RegisterPrefix(TokenType::kBang, parse_prefix);
  RegisterPrefix(TokenType::kMinus, parse_prefix);

  auto parse_infix = [this](const auto& expr) {
    return ParseInfixExpression(expr);
  };
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
                [this](const auto& expr) { return ParseCallExpression(expr); });
}

void Parser::NextToken() {
  curr_token_ = peek_token_;
  peek_token_ = lexer_.NextToken();
}

Program Parser::ParseProgram() {
  Program program;
  while (curr_token_.type != TokenType::kEof) {
    const Statement stmt = ParseStatement();
    if (stmt.Ok()) {
      program.statements.push_back(stmt);
    }
    NextToken();
  }
  return program;
}

std::string Parser::ErrorMsg() const { return absl::StrJoin(errors_, "\n"); }

Statement Parser::ParseStatement() {
  switch (curr_token_.type) {
    case TokenType::kLet:
      return ParseLetStatement();
    case TokenType::kReturn:
      return ParseReturnStatement();
    default:
      return ParseExpressionStatement();
  }
}

Statement Parser::ParseLetStatement() {
  LetStatement let_stmt;
  let_stmt.token = curr_token_;

  if (!ExpectPeek(TokenType::kIdent)) {
    LOG(INFO) << "[ParseLetStatement] Next token is not Ident";
    return StatementBase{};
  }

  let_stmt.name.token = curr_token_;
  let_stmt.name.value = curr_token_.literal;

  if (!ExpectPeek(TokenType::kAssign)) {
    LOG(INFO) << "[ParseLetStatement] Next token is not Assing";
    return StatementBase{};
  }

  NextToken();
  let_stmt.expr = ParseExpression(Precedence::kLowest);

  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return let_stmt;
}

Statement Parser::ParseReturnStatement() {
  ReturnStatement ret_stmt;
  ret_stmt.token = curr_token_;

  NextToken();
  ret_stmt.expr = ParseExpression(Precedence::kLowest);

  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return ret_stmt;
}

Statement Parser::ParseExpressionStatement() {
  ExpressionStatement expr_stmt;
  expr_stmt.token = curr_token_;
  expr_stmt.expr = ParseExpression(Precedence::kLowest);

  if (IsPeekToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return expr_stmt;
}

BlockStatement Parser::ParseBlockStatement() {
  BlockStatement block_stmt;
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

Expression Parser::ParseExpression(Precedence precedence) {
  const auto prefix_it = prefix_parse_fn_.find(curr_token_.type);
  if (prefix_it == prefix_parse_fn_.end()) {
    const std::string msg =
        fmt::format("no prefix parse function for {}", curr_token_.type);
    LOG(WARNING) << msg;
    errors_.push_back(msg);
    return ExpressionBase{};
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

Expression Parser::ParseIdentifier() {
  Identifier ident;
  ident.token = curr_token_;
  ident.value = curr_token_.literal;
  return ident;
}

Expression Parser::ParseIntegerLiteral() {
  IntLiteral int_lit;
  int_lit.token = curr_token_;

  bool ok = absl::SimpleAtoi(int_lit.token.literal, &int_lit.value);
  if (!ok) {
    const auto msg =
        fmt::format("could not parse {} as integer", curr_token_.literal);
    LOG(WARNING) << msg;
    errors_.push_back(msg);
    return ExpressionBase{};
  }

  return int_lit;
}

Expression Parser::ParseBooleanLiteral() {
  BoolLiteral bool_lit;
  bool_lit.token = curr_token_;
  bool_lit.value = IsCurrToken(TokenType::kTrue);
  return bool_lit;
}

Expression Parser::ParseFunctionLiteral() {
  FunctionLiteral fn_lit;
  fn_lit.token = curr_token_;

  if (!ExpectPeek(TokenType::kLParen)) {
    return ExpressionBase{};
  }

  fn_lit.params = ParseFunctionParameters();

  if (!ExpectPeek(TokenType::kLBrace)) {
    return ExpressionBase{};
  }

  fn_lit.body = ParseBlockStatement();
  return fn_lit;
}

Expression Parser::ParseInfixExpression(const Expression& lhs) {
  InfixExpression infx_expr;
  infx_expr.token = curr_token_;
  infx_expr.op = curr_token_.literal;
  infx_expr.lhs = lhs;

  const auto precedence = CurrPrecedence();
  NextToken();
  infx_expr.rhs = ParseExpression(precedence);
  return infx_expr;
}

Expression Parser::ParseGroupedExpression() {
  NextToken();
  auto expr = ParseExpression(Precedence::kLowest);
  if (!ExpectPeek(TokenType::kRParen)) {
    return ExpressionBase{};
  }
  return expr;
}

Expression Parser::ParseIfExpression() {
  IfExpression if_expr;
  if_expr.token = curr_token_;

  if (!ExpectPeek(TokenType::kLParen)) {
    return ExpressionBase{};
  }

  NextToken();
  if_expr.cond = ParseExpression(Precedence::kLowest);

  if (!ExpectPeek(TokenType::kRParen)) {
    return ExpressionBase{};
  }
  if (!ExpectPeek(TokenType::kLBrace)) {
    return ExpressionBase{};
  }
  if_expr.true_block = ParseBlockStatement();

  // else
  if (IsPeekToken(TokenType::kElse)) {
    NextToken();
    if (!ExpectPeek(TokenType::kLBrace)) {
      return ExpressionBase{};
    }
    if_expr.false_block = ParseBlockStatement();
  }

  return if_expr;
}

Expression Parser::ParseCallExpression(const Expression& expr) {
  CallExpression call_expr;
  call_expr.token = curr_token_;
  call_expr.func = expr;
  call_expr.args = ParseCallArguments();
  return call_expr;
}

std::vector<Identifier> Parser::ParseFunctionParameters() {
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

std::vector<Expression> Parser::ParseCallArguments() {
  std::vector<Expression> args;

  if (IsPeekToken(TokenType::kRParen)) {
    NextToken();
    return args;
  }

  NextToken();
  args.push_back(ParseExpression(Precedence::kLowest));

  while (IsPeekToken(TokenType::kComma)) {
    NextToken();
    NextToken();
    args.push_back(ParseExpression(Precedence::kLowest));
  }

  if (!ExpectPeek(TokenType::kRParen)) {
    return {};
  }

  return args;
}

Expression Parser::ParsePrefixExpression() {
  PrefixExpression prefix_expr;
  prefix_expr.token = curr_token_;
  prefix_expr.op = curr_token_.literal;

  NextToken();
  prefix_expr.rhs = ParseExpression(Precedence::kPrefix);

  return prefix_expr;
}

bool Parser::ExpectPeek(TokenType type) {
  if (IsPeekToken(type)) {
    NextToken();
    return true;
  } else {
    PeekError(type);
    return false;
  }
}

void Parser::PeekError(TokenType type) {
  errors_.push_back(fmt::format(
      "Expected next token to be {}, got {} instead", type, peek_token_.type));
}

Precedence Parser::TokenPrecedence(TokenType type) const {
  const auto it = gTokenPrecedence.find(type);
  if (it != gTokenPrecedence.end()) {
    return it->second;
  }
  return Precedence::kLowest;
}

Precedence Parser::CurrPrecedence() const {
  return TokenPrecedence(curr_token_.type);
}

Precedence Parser::PeekPrecedence() const {
  return TokenPrecedence(peek_token_.type);
}

}  // namespace monkey
