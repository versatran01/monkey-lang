#pragma once

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
  kBuiltinFunc,
  kArray,
};

std::ostream& operator<<(std::ostream& os, ObjectType type);

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

  ObjectType type{ObjectType::kInvalid};
  absl::any value;
};

using BuiltinFunc = std::function<Object(std::vector<Object>)>;
using Array = std::vector<Object>;

struct FuncObject {
  std::string Inspect() const;

  std::vector<Identifier> params;
  BlockStmt body;
  std::shared_ptr<Environment> env{nullptr};
};

// Use these to create new objects
Object NullObj();
Object IntObj(int64_t value);
Object StrObj(std::string value);
Object BoolObj(bool value);
Object ErrorObj(std::string value);
Object ReturnObj(Object value);
Object FuncObj(FuncObject value);
Object BuiltinFuncObj(BuiltinFunc value);
Object ArrayObj(Array value);

}  // namespace monkey
