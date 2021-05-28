#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/types/optional.h>

#include <string>

namespace monkey {

using SymbolScope = std::string;
static const SymbolScope kGlobalScope = "GLOBAL";
static const SymbolScope kLocalScope = "LOCAL";

struct Symbol {
  std::string name;
  SymbolScope scope;
  int index;

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);

  friend bool operator==(const Symbol& lhs, const Symbol& rhs) {
    return lhs.index == rhs.index && lhs.scope == rhs.scope &&
           lhs.name == rhs.name;
  }

  friend bool operator!=(const Symbol& lhs, const Symbol& rhs) {
    return !(lhs == rhs);
  }
};

using SymbolDict = absl::flat_hash_map<std::string, Symbol>;

class SymbolTable {
 public:
  Symbol& Define(const std::string& name);
  absl::optional<Symbol> Resolve(const std::string& name) const;

  int NumDefs() const noexcept { return num_defs_; }

 private:
  SymbolDict store_;
  int num_defs_{0};
};

}  // namespace monkey
