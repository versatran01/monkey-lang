#include "monkey/token.h"

#include <absl/container/flat_hash_map.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

namespace {

const auto gKeywords =
    absl::flat_hash_map<std::string, TokenType>{{"let", TokenType::kLet},
                                                {"fn", TokenType::kFunc},
                                                {"if", TokenType::kIf},
                                                {"else", TokenType::kElse},
                                                {"true", TokenType::kTrue},
                                                {"false", TokenType::kFalse},
                                                {"return", TokenType::kReturn}};

const auto gTokenTypeStrings = absl::flat_hash_map<TokenType, std::string>{
    {TokenType::kIllegal, "ILLEGAL"}, {TokenType::kEof, "EOF"},
    {TokenType::kIdent, "IDENT"},     {TokenType::kInt, "INT"},
    {TokenType::kStr, "STRING"},      {TokenType::kAssign, "="},
    {TokenType::kPlus, "+"},          {TokenType::kMinus, "-"},
    {TokenType::kBang, "!"},          {TokenType::kAsterisk, "*"},
    {TokenType::kSlash, "/"},         {TokenType::kLt, "<"},
    {TokenType::kLe, "<="},           {TokenType::kGt, ">"},
    {TokenType::kGe, ">="},           {TokenType::kEq, "=="},
    {TokenType::kNe, "!="},           {TokenType::kComma, ","},
    {TokenType::kSemicolon, ";"},     {TokenType::kLParen, "("},
    {TokenType::kRParen, ")"},        {TokenType::kLBrace, "{"},
    {TokenType::kRBrace, "}"},        {TokenType::kLBracket, "["},
    {TokenType::kRBracket, "]"},      {TokenType::kFunc, "FN"},
    {TokenType::kLet, "LET"},         {TokenType::kTrue, "TRUE"},
    {TokenType::kFalse, "FALSE"},     {TokenType::kIf, "IF"},
    {TokenType::kElse, "ELSE"},       {TokenType::kReturn, "RETURN"},
    {TokenType::kColon, "COLON"},
};

}  // namespace

std::string Repr(TokenType type) {
  CHECK(gTokenTypeStrings.contains(type)) << type;
  return gTokenTypeStrings.at(type);
}

std::ostream& operator<<(std::ostream& os, TokenType type) {
  return os << Repr(type);
}

TokenType GetKeywordType(absl::string_view name) {
  const auto it = gKeywords.find(name);
  if (it != gKeywords.cend()) {
    return it->second;
  }
  return TokenType::kIdent;
}

std::string Token::Repr() const {
  return fmt::format("Token({}, {})", type, literal);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  return os << token.Repr();
}

}  // namespace monkey
