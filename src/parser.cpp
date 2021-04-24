#include "monkey/parser.h"

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
    Node stmt = ParseStatement();
    if (stmt.Type() != NodeType::kBase) {
      program.statements.push_back(stmt);
    }
    NextToken();
  }
  return program;
}

Node Parser::ParseStatement() {
  switch (curr_token_.type) {
    case TokenType::kLet:
      return ParseLetStatement();
    default:
      return NodeBase{};
  }
}

Node Parser::ParseLetStatement() {
  LetStatement stmt;
  stmt.token = curr_token_;

  if (!ExpectPeek(TokenType::kIdent)) {
    return NodeBase{};
  }

  Identifier ident;
  ident.token = curr_token_;
  ident.value = curr_token_.literal;
  stmt.name = ident;

  if (!ExpectPeek(TokenType::kAssign)) {
    return NodeBase{};
  }

  // TODO: we're skipping the expressions until we encounter a semicolon
  while (!IsCurrToken(TokenType::kSemicolon)) {
    NextToken();
  }

  return stmt;
}

bool Parser::ExpectPeek(TokenType type) {
  if (IsPeekToken(type)) {
    NextToken();
    return true;
  }
  return false;
}

}  // namespace monkey
