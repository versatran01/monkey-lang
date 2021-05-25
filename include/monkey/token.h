#pragma once

#include <absl/strings/string_view.h>

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

std::string Repr(TokenType type);
std::ostream& operator<<(std::ostream& os, TokenType type);

struct Token {
  TokenType type{TokenType::kIllegal};
  std::string literal;

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

/// Get the type of the keyword, if not a keyword then type is identifier
TokenType GetKeywordType(absl::string_view name);

}  // namespace monkey
