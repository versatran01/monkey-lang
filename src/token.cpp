#include "monkey/token.h"

#include <absl/container/flat_hash_map.h>

namespace monkey {

namespace {

const absl::flat_hash_map<std::string, TokenType> keywords = {
    {"let", token_type::kLet}, {"fn", token_type::kFunction}};

}

TokenType LookupIdentifier(const std::string &ident) {
  const auto it = keywords.find(ident);
  if (it != keywords.end()) {
    return it->second;
  }
  return token_type::kIdent;
}

}  // namespace monkey
