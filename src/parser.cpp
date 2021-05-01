#include "monkey/parser.h"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {
  // Read two tokens, so curToken and peekToken are both set
  NextToken();
  NextToken();
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

StmtNode Parser::ParseStatement() {
  switch (curr_token_.type) {
    case TokenType::kLet:
      return ParseLetStatement();
    case TokenType::kReturn:
      return ParseReturnStatement();
    default:
      return NodeBase{};
  }
}

StmtNode Parser::ParseLetStatement() {
  LetStatement stmt;
  stmt.token = curr_token_;

  if (!ExpectPeek(TokenType::kIdent)) {
    return NodeBase{};
  }

  stmt.name.token = curr_token_;
  stmt.name.value = curr_token_.literal;

  if (!ExpectPeek(TokenType::kAssign)) {
    return NodeBase{};
  }

  // TODO: we're skipping the expressions until we encounter a semicolon
  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return stmt;
}

StmtNode Parser::ParseReturnStatement() {
  ReturnStatement stmt;
  stmt.token = curr_token_;

  NextToken();

  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return stmt;
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
