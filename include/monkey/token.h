#pragma once

#include <iosfwd>
#include <string>

namespace monkey {

enum class TokenType {
  kIllegal,
  kEof,
  kIdent,
  kInt,
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
  kFunc,
  kLet,
  kTrue,
  kFalse,
  kIf,
  kElse,
  kReturn
};

std::ostream& operator<<(std::ostream& os, TokenType type);

struct Token {
  Token() noexcept = default;

  TokenType type{TokenType::kIllegal};
  std::string literal;

  friend std::ostream& operator<<(std::ostream& os, Token token);
};

TokenType LookupIdentifier(const std::string& ident);

}  // namespace monkey
