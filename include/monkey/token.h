#pragma once

#include <string>

namespace monkey {

using TokenType = std::string;

namespace token_type {

const TokenType kIllegal = "ILLEGAL";  // a token we don't know
const TokenType kEof = "EOF";

const TokenType kIdent = "IDENT";  // identifier
const TokenType kInt = "INT";

const TokenType kAssign = "=";
const TokenType kPlus = "+";

const TokenType kComma = ",";
const TokenType kSemicolon = ";";

const TokenType kLParen = "(";
const TokenType kRParen = ")";
const TokenType kLBrace = "{";
const TokenType kRBrace = "}";

// keywords
const TokenType kFunction = "FUNCTION";
const TokenType kLet = "LET";

}  // namespace token_type

class Token {
 public:
  Token() = default;

  TokenType type;
  std::string literal;
};

}  // namespace monkey
