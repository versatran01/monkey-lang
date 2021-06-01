#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/types/optional.h>

#include <memory>
#include <string>

namespace monkey {

enum class SymbolScope {
  kGlobal,
  kLocal,
  kBuiltin,
  kFree,
};

std::string Repr(SymbolScope scope);
std::ostream& operator<<(std::ostream& os, SymbolScope scope);

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
  explicit SymbolTable(SymbolTable* outer = nullptr) : outer_{outer} {}

  Symbol& Define(const std::string& name);
  Symbol& DefineBuiltin(const std::string& name, size_t index);
  Symbol& DefineFree(const Symbol& symbol);
  absl::optional<Symbol> Resolve(const std::string& name);

  size_t NumDefs() const noexcept { return num_defs_; }
  size_t NumFree() const noexcept { return free_symbols_.size(); }
  bool IsGlobal() const noexcept { return outer_ == nullptr; }
  const auto& FreeSymbols() const noexcept { return free_symbols_; }

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const SymbolTable& table);

 private:
  SymbolDict store_;
  size_t num_defs_{0};
  std::vector<Symbol> free_symbols_;
  SymbolTable* outer_{nullptr};
};

using SymbolTablePtr = std::unique_ptr<SymbolTable>;

}  // namespace monkey
