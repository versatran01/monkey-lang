#include "monkey/symbol.h"

#include <fmt/core.h>

namespace monkey {

std::string Symbol::Repr() const {
  return fmt::format("Symbol(name={}, scope={}, ind={})", name, scope, index);
}

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
  return os << symbol.Repr();
}

Symbol& SymbolTable::Define(const std::string& name) {
  return store_[name] = {name, kGlobalScope, num_defs_++};
}

absl::optional<Symbol> SymbolTable::Resolve(const std::string& name) const {
  const auto it = store_.find(name);
  if (it == store_.end()) return absl::nullopt;
  return it->second;
}

}  // namespace monkey
