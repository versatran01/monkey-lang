#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/types/any.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "monkey/ast.h"

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
  kBuiltin,
  kArray,
  kDict,
  kQuote,
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
  explicit Object(ObjectType type) : type{type} {}
  Object(ObjectType type, const absl::any& value) : type{type}, value{value} {}
  virtual ~Object() noexcept = default;

  std::string Inspect() const;
  ObjectType Type() const noexcept { return type; }
  bool Ok() const noexcept { return value.has_value(); }
  friend std::ostream& operator<<(std::ostream& os, const Object& obj);

  template <typename T>
  auto Cast() const {
    return absl::any_cast<const T&>(value);
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
using Builtin = std::function<Object(std::vector<Object>)>;

struct FuncObject {
  std::string Inspect() const;

  std::vector<Identifier> params;
  BlockStmt body;
  std::shared_ptr<Environment> env{nullptr};
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
Object ReturnObj(const Object& obj);
Object FuncObj(const FuncObject& fn);
Object BuiltinObj(const Builtin& fn);
Object ArrayObj(Array arr);
Object DictObj(Dict dict);
Object QuoteObj(const ExprNode& expr);

// Directly create object from ast node
Object ToIntObj(const ExprNode& expr);
Object ToBoolObj(const ExprNode& expr);
Object ToStrObj(const ExprNode& expr);

}  // namespace monkey
