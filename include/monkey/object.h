#pragma once

#include <string>

#include "monkey/te.hpp"

namespace monkey {

enum class ObjectType { kNull, kInt, kBool };

std::ostream &operator<<(std::ostream &os, ObjectType type);

struct ObjectInterface {
  auto Inspect() const {
    return boost::te::call<std::string>(
        [](const auto &self) { return self.Inspect(); }, *this);
  }

  auto Type() const {
    return boost::te::call<ObjectType>(
        [](const auto &self) { return self.Type(); }, *this);
  }
};

using Object = boost::te::poly<ObjectInterface>;

struct ObjectBase {
  explicit ObjectBase(ObjectType type) : type{type} {}
  virtual ~ObjectBase() noexcept = default;

  std::string Inspect() const { return InspectImpl(); }
  ObjectType Type() const noexcept { return type; }
  virtual std::string InspectImpl() const = 0;

  ObjectType type;
};

struct NullObject final : public ObjectBase {
  NullObject() : ObjectBase{ObjectType::kNull} {}
  std::string InspectImpl() const override { return "null"; }
};

struct IntObject final : public ObjectBase {
  using ValueType = int64_t;

  IntObject() : ObjectBase{ObjectType::kInt} {}
  std::string InspectImpl() const override { return std::to_string(value); }

  ValueType value{};
};

struct BoolObject final : public ObjectBase {
  using ValueType = bool;

  BoolObject() : ObjectBase{ObjectType::kBool} {}
  std::string InspectImpl() const override { return value ? "true" : "false"; }

  ValueType value{};
};

}  // namespace monkey
