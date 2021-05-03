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
};

}

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {
  // Read two tokens, so curToken and peekToken are both set
  NextToken();
  NextToken();
  RegisterParseFns();
}

void Parser::RegisterParseFns() {
  RegisterPrefix(TokenType::kIdent, [this]() { return ParseIdentifier(); });
  RegisterPrefix(TokenType::kInt, [this]() { return ParseIntegerLiteral(); });
  RegisterPrefix(TokenType::kTrue, [this]() { return ParseBooleanLiteral(); });
  RegisterPrefix(TokenType::kFalse, [this]() { return ParseBooleanLiteral(); });

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
  LetStatement stmt;
  stmt.token = curr_token_;

  if (!ExpectPeek(TokenType::kIdent)) {
    LOG(INFO) << "[ParseLetStatement] Next token is not Ident";
    return StatementBase{};
  }

  stmt.name.token = curr_token_;
  stmt.name.value = curr_token_.literal;

  if (!ExpectPeek(TokenType::kAssign)) {
    LOG(INFO) << "[ParseLetStatement] Next token is not Assing";
    return StatementBase{};
  }

  // TODO: we're skipping the expressions until we encounter a semicolon
  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return stmt;
}

Statement Parser::ParseReturnStatement() {
  ReturnStatement stmt;
  stmt.token = curr_token_;

  NextToken();

  // TODO: we're skipping the expressions until we encounter a semicolon
  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return stmt;
}

Statement Parser::ParseExpressionStatement() {
  ExpressionStatement stmt;
  stmt.token = curr_token_;
  stmt.expr = ParseExpression(Precedence::kLowest);

  if (IsPeekToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return stmt;
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

  return lhs;  // call the function
}

Expression Parser::ParseIdentifier() {
  Identifier ident;
  ident.token = curr_token_;
  ident.value = curr_token_.literal;
  return ident;
}

Expression Parser::ParseIntegerLiteral() {
  IntegerLiteral expr;
  expr.token = curr_token_;

  bool ok = absl::SimpleAtoi(expr.token.literal, &expr.value);
  if (!ok) {
    const auto msg =
        fmt::format("could not parse {} as integer", curr_token_.literal);
    LOG(WARNING) << msg;
    errors_.push_back(msg);
    return ExpressionBase{};
  }

  return expr;
}

Expression Parser::ParseBooleanLiteral() {
  BooleanLiteral expr;
  expr.token = curr_token_;
  expr.value = IsCurrToken(TokenType::kTrue);
  return expr;
}

Expression Parser::ParseInfixExpression(const Expression& lhs) {
  InfixExpression expr;
  expr.token = curr_token_;
  expr.op = curr_token_.literal;
  expr.lhs = lhs;

  const auto precedence = CurrPrecedence();
  NextToken();
  expr.rhs = ParseExpression(precedence);
  return expr;
}

Expression Parser::ParsePrefixExpression() {
  PrefixExpression expr;
  expr.token = curr_token_;
  expr.op = curr_token_.literal;

  NextToken();
  expr.rhs = ParseExpression(Precedence::kPrefix);

  return expr;
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
  std::string msg = fmt::format("Expected next token to be {}, got {} instead",
                                type, peek_token_.type);
  errors_.push_back(std::move(msg));
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
