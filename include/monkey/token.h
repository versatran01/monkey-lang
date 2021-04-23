#pragma once

#include <string>

namespace monkey {

using TokenType = std::string;

namespace token_type {

const TokenType kIllegal = "ILLEGAL";  // a token we don't know
const TokenType kEof = "EOF";

const TokenType kIdent = "IDENT";  // identifier
const TokenType kInt = "INT";

// Opeartors
const TokenType kAssign = "=";
const TokenType kPlus = "+";
const TokenType kMinus = "-";
const TokenType kBang = "!";
const TokenType kAsterisk = "*";
const TokenType kSlash = "/";
const TokenType kLt = "<";
const TokenType kLe = "<=";
const TokenType kGt = ">";
const TokenType kGe = ">=";
const TokenType kEq = "==";
const TokenType kNe = "!=";

const TokenType kComma = ",";
const TokenType kSemicolon = ";";

const TokenType kLParen = "(";
const TokenType kRParen = ")";
const TokenType kLBrace = "{";
const TokenType kRBrace = "}";

// keywords
const TokenType kFunction = "FUNCTION";
const TokenType kLet = "LET";
const TokenType kTrue = "TRUE";
const TokenType kFalse = "FALSE";
const TokenType kIf = "IF";
const TokenType kElse = "ELSE";
const TokenType kReturn = "RETURN";

}  // namespace token_type

class Token {
 public:
  Token() = default;

  TokenType type;
  std::string literal;
};

TokenType LookupIdentifier(const std::string& ident);

}  // namespace monkey
