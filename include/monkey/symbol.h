#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/types/optional.h>

#include <memory>
#include <string>

namespace monkey {

enum class SymbolScope {
  kGlobal,
  kLocal,
};

struct Symbol {
  std::string name;
  SymbolScope scope;
  size_t index;

  bool IsGlobal() const noexcept { return scope == SymbolScope::kGlobal; }

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
  explicit SymbolTable(const SymbolTable* outer = nullptr) : outer_{outer} {}

  Symbol& Define(const std::string& name);
  absl::optional<Symbol> Resolve(const std::string& name) const;
  bool IsGlobal() const noexcept { return outer_ == nullptr; }

  size_t NumDefs() const noexcept { return num_defs_; }

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const SymbolTable& table);

 private:
  SymbolDict store_;
  size_t num_defs_{0};
  const SymbolTable* outer_;
};

using SymbolTablePtr = std::unique_ptr<SymbolTable>;

}  // namespace monkey
