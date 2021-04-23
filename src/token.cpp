#include "monkey/token.h"

#include <absl/container/flat_hash_map.h>

namespace monkey {

namespace {

const absl::flat_hash_map<std::string, TokenType> keywords = {
    {"let", token_type::kLet},      {"fn", token_type::kFunction},
    {"if", token_type::kLet},       {"else", token_type::kElse},
    {"true", token_type::kTrue},    {"false", token_type::kFalse},
    {"return", token_type::kReturn}};

}

TokenType LookupIdentifier(const std::string &ident) {
  const auto it = keywords.find(ident);
  if (it != keywords.end()) {
    return it->second;
  }
  return token_type::kIdent;
}

}  // namespace monkey
