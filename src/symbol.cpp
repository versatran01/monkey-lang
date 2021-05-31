#include "monkey/symbol.h"

#include <absl/strings/str_join.h>
#include <fmt/core.h>

namespace monkey {
namespace {
struct SymbolFmt {
  void operator()(std::string* out, const Symbol& symbol) const {
    out->append(symbol.Repr());
  }
};
}  // namespace

std::string Symbol::Repr() const {
  return fmt::format(
      "Symbol({}, {}, {})", name, IsGlobal() ? "global" : "local", index);
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

absl::optional<Symbol> SymbolTable::Resolve(const std::string& name) const {
  const auto it = store_.find(name);

  if (it == store_.end()) {
    // not found in global scope
    if (outer_ == nullptr) return absl::nullopt;
    // try the outer scope
    return outer_->Resolve(name);
  }
  return it->second;
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
