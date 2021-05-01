#include "monkey/parser.h"

#include <absl/strings/numbers.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {
  // Read two tokens, so curToken and peekToken are both set
  NextToken();
  NextToken();
  RegisterParseFns();
}

void Parser::RegisterParseFns() {
  RegisterPrefix(TokenType::kIdent, [this]() { return ParseIdentifier(); });
  RegisterPrefix(TokenType::kInt, [this]() { return ParseIntegerLiteral(); });
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
  const auto it = prefix_parse_fn_.find(curr_token_.type);
  if (it == prefix_parse_fn_.end()) {
    return ExpressionBase{};
  }

  return it->second();  // call the function
}

Expression Parser::ParseIdentifier() {
  Identifier ident;
  ident.token = curr_token_;
  ident.value = curr_token_.literal;
  return ident;
}

Expression Parser::ParseIntegerLiteral() {
  IntegerLiteral lit;
  lit.token = curr_token_;

  bool ok = absl::SimpleAtoi(lit.token.literal, &lit.value);
  if (!ok) {
    const auto msg =
        fmt::format("could not parse {} as integer", curr_token_.literal);
    LOG(WARNING) << msg;
    errors_.push_back(msg);
    return ExpressionBase{};
  }

  return lit;
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

}  // namespace monkey
