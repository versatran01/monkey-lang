#include "monkey/token.h"

#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <ostream>

namespace monkey {

namespace {

const absl::flat_hash_map<std::string, TokenType> gKeywords = {
    {"let", TokenType::kLet},      {"fn", TokenType::kFunction},
    {"if", TokenType::kLet},       {"else", TokenType::kElse},
    {"true", TokenType::kTrue},    {"false", TokenType::kFalse},
    {"return", TokenType::kReturn}};

const absl::flat_hash_map<TokenType, std::string> gTokenTypeStrings{
    {TokenType::kIllegal, "ILLEGAL"},
    {TokenType::kEof, "EOF"},
    {TokenType::kIdent, "IDENT"},
    {TokenType::kInt, "INT"},
    {TokenType::kAssign, "="},
    {TokenType::kPlus, "+"},
    {TokenType::kMinus, "-"},
    {TokenType::kBang, "!"},
    {TokenType::kAsterisk, "*"},
    {TokenType::kSlash, "/"},
    {TokenType::kLt, "<"},
    {TokenType::kLe, "<="},
    {TokenType::kGt, ">"},
    {TokenType::kGe, ">="},
    {TokenType::kEq, "=="},
    {TokenType::kNe, "!="},
    {TokenType::kComma, ","},
    {TokenType::kSemicolon, ";"},
    {TokenType::kLParen, "("},
    {TokenType::kRParen, ")"},
    {TokenType::kLBrace, "{"},
    {TokenType::kRBrace, "}"},
    {TokenType::kFunction, "FUNCTION"},
    {TokenType::kLet, "LET"},
    {TokenType::kTrue, "TRUE"},
    {TokenType::kFalse, "FALSE"},
    {TokenType::kIf, "IF"},
    {TokenType::kElse, "ELSE"},
    {TokenType::kReturn, "RETURN"}};

}  // namespace

TokenType LookupIdentifier(const std::string& ident) {
  const auto it = gKeywords.find(ident);
  if (it != gKeywords.end()) {
    return it->second;
  }
  return TokenType::kIdent;
}

std::ostream& operator<<(std::ostream& os, TokenType token_type) {
  return os << gTokenTypeStrings.at(token_type);
}

std::ostream& operator<<(std::ostream& os, Token token) {
  return os << fmt::format("Token({}, {})", token.type, token.literal);
}

}  // namespace monkey
