#include "monkey/symbol.h"

#include <absl/strings/str_join.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

namespace {
struct SymbolFmt {
  void operator()(std::string* out, const Symbol& symbol) const {
    out->append(symbol.Repr());
  }
};

}  // namespace

std::string Repr(SymbolScope scope) {
  switch (scope) {
    case SymbolScope::kBuiltin:
      return "builtin";
    case SymbolScope::kGlobal:
      return "global";
    case SymbolScope::kLocal:
      return "local";
    case SymbolScope::kFree:
      return "free";
    default:
      return "unknown scope";
  }
}

std::ostream& operator<<(std::ostream& os, SymbolScope scope) {
  return os << Repr(scope);
}

std::string Symbol::Repr() const {
  return fmt::format("Symbol({}, {}, {})", name, scope, index);
}

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
  return os << symbol.Repr();
}

Symbol& SymbolTable::Define(const std::string& name) {
  return store_[name] = {
             name,
             IsGlobal() ? SymbolScope::kGlobal : SymbolScope::kLocal,
             num_defs_++};
}

Symbol& SymbolTable::DefineBuiltin(const std::string& name, size_t index) {
  return store_[name] = {name, SymbolScope::kBuiltin, index};
}

Symbol& SymbolTable::DefineFree(const Symbol& symbol) {
  free_symbols_.push_back(symbol);

  return store_[symbol.name] =
             Symbol{symbol.name, SymbolScope::kFree, NumFree() - 1};
}

absl::optional<Symbol> SymbolTable::Resolve(const std::string& name) {
  // Is it found in the current scope?
  const auto it = store_.find(name);
  // Yes, return
  if (it != store_.end()) return it->second;

  // This is the global scope, and not found, return nullopt
  if (outer_ == nullptr) return absl::nullopt;

  // There is an outer scope, look in there
  auto res = outer_->Resolve(name);
  // Not found, return nullopt
  if (!res.has_value()) return absl::nullopt;

  // Is it found in global scope or builtin scope?
  if (res->scope == SymbolScope::kGlobal ||
      res->scope == SymbolScope::kBuiltin) {
    return res;
  }

  // Otherwise this is a free symbol
  return DefineFree(*res);
}

std::string SymbolTable::Repr() const {
  auto pf = absl::PairFormatter(absl::AlphaNumFormatter(), ": ", SymbolFmt{});
  return fmt::format("{}{{{}}}",
                     IsGlobal() ? "Global" : "Local",
                     absl::StrJoin(store_, ", ", pf));
}

std::ostream& operator<<(std::ostream& os, const SymbolTable& table) {
  return os << table.Repr();
}

}  // namespace monkey
