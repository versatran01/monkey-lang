#pragma once

#include <absl/types/any.h>

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
  kReturn,
  kError,
  kFunction,
};

std::ostream& operator<<(std::ostream& os, ObjectType type);

struct Object {
  Object() = default;
  explicit Object(ObjectType type) : type{type} {}
  Object(ObjectType type, const absl::any& value) : type{type}, value{value} {}
  virtual ~Object() noexcept = default;

  std::string Inspect() const;
  ObjectType Type() const noexcept { return type; }
  friend std::ostream& operator<<(std::ostream& os, const Object& obj);

  template <typename T>
  auto CastCRef() const {
    return absl::any_cast<const T&>(value);
  }

  ObjectType type{ObjectType::kInvalid};
  absl::any value;
};

struct FnObject {
  std::string Inspect() const;

  std::vector<Identifier> params;
  BlockStatement body;
  Environment* env{nullptr};
};

Object NullObject();
Object IntObject(int64_t value);
Object BoolObject(bool value);
Object ErrorObject(std::string value);
Object FunctionObject(FnObject value);

}  // namespace monkey
