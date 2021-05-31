#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/types/any.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "monkey/ast.h"
#include "monkey/instruction.h"

namespace monkey {

class Environment;

enum class ObjectType {
  kInvalid,
  kNull,
  kInt,
  kBool,
  kStr,
  kReturn,
  kError,
  kFunc,
  kArray,
  kDict,
  kQuote,
  kBuiltinFunc,
  kCompiled,
};

std::string Repr(ObjectType type);
std::ostream& operator<<(std::ostream& os, ObjectType type);

// Check if objects are the same type
template <typename... Args>
bool ObjOfSameType(ObjectType op, Args const&... args) {
  return ((args.Type() == op) && ... && true);
}

struct Object {
  Object() = default;
  Object(const Object&) = default;
  Object& operator=(const Object&) = default;
  Object(Object&&) = default;
  Object& operator=(Object&&) = default;

  explicit Object(ObjectType type) : type{type} {}
  Object(ObjectType type, absl::any value)
      : type{type}, value{std::move(value)} {}

  virtual ~Object() noexcept = default;

  std::string Inspect() const;
  ObjectType Type() const noexcept { return type; }
  bool Ok() const noexcept { return value.has_value(); }
  friend std::ostream& operator<<(std::ostream& os, const Object& obj);

  template <typename T>
  const T& Cast() const {
    return absl::any_cast<const T&>(value);
  }

  template <typename T>
  T& MutCast() {
    return absl::any_cast<T&>(value);
  }

  // https://abseil.io/docs/cpp/guides/hash
  template <typename H>
  friend H AbslHashValue(H h, const Object& obj) {
    const auto t = static_cast<int>(obj.Type());
    switch (obj.Type()) {
      case ObjectType::kBool:
        return H::combine(std::move(h), t, absl::any_cast<BoolType>(obj.value));
      case ObjectType::kInt:
        return H::combine(std::move(h), t, absl::any_cast<IntType>(obj.value));
      case ObjectType::kStr:
        return H::combine(std::move(h), t, absl::any_cast<StrType>(obj.value));
      default:
        return H::combine(std::move(h), t, obj.Inspect());
    }
  }

  friend bool operator==(const Object& lhs, const Object& rhs);

  friend bool operator!=(const Object& lhs, const Object& rhs) {
    return !(lhs == rhs);
  }

  ObjectType type{ObjectType::kInvalid};
  absl::any value;
};

using Array = std::vector<Object>;
using Dict = absl::flat_hash_map<Object, Object>;

struct BuiltinFunc {
  std::string name;
  std::function<Object(const std::vector<Object>&)> func;
};

struct FuncObject {
  std::string Inspect() const;

  std::vector<Identifier> params;
  BlockStmt body;
  std::shared_ptr<Environment> env{nullptr};
};

struct CompiledFunc {
  std::string Inspect() const;

  Instruction ins;
  size_t num_locals{0};
  size_t num_params{0};
};

bool IsObjTruthy(const Object& obj);
bool IsObjError(const Object& obj) noexcept;
bool IsObjHashable(const Object& obj) noexcept;

// Use these to create new objects
Object NullObj();
Object IntObj(IntType value);
Object StrObj(StrType value);
Object BoolObj(BoolType value);
Object ErrorObj(StrType str);
Object ReturnObj(Object obj);
Object ArrayObj(Array arr);
Object DictObj(Dict dict);
Object BuiltinObj(const BuiltinFunc& fn);
Object FuncObj(const FuncObject& fn);
Object QuoteObj(const ExprNode& expr);
Object CompiledObj(CompiledFunc comp);
Object CompiledObj(const std::vector<Instruction>& ins);

// Directly create object from ast node
Object ToIntObj(const ExprNode& expr);
Object ToBoolObj(const ExprNode& expr);
Object ToStrObj(const ExprNode& expr);

}  // namespace monkey
