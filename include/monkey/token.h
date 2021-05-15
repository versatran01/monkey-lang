#pragma once

#include <iosfwd>
#include <string>

namespace monkey {

enum class TokenType {
  kIllegal,
  kEof,
  kIdent,
  kInt,
  kStr,
  kAssign,
  kPlus,
  kMinus,
  kBang,
  kAsterisk,
  kSlash,
  kLt,
  kLe,
  kGt,
  kGe,
  kEq,
  kNe,
  kComma,
  kSemicolon,
  kLParen,
  kRParen,
  kLBrace,
  kRBrace,
  kLBracket,
  kRBracket,
  kFunc,
  kLet,
  kTrue,
  kFalse,
  kIf,
  kElse,
  kReturn,
  kColon
};

std::ostream& operator<<(std::ostream& os, TokenType type);

struct Token {
  TokenType type{TokenType::kIllegal};
  std::string literal;

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

TokenType GetKeywordType(const std::string& ident);

}  // namespace monkey
