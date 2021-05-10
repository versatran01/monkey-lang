#pragma once

#include <string>
#include <vector>

#include "monkey/te.hpp"

namespace monkey {

enum class ObjectType {
  kInvalid,
  kNull,
  kInt,
  kBool,
  kReturn,
  kError,
  kFunction
};

std::ostream& operator<<(std::ostream& os, ObjectType type);

struct ObjectBase {
  ObjectBase() noexcept = default;
  explicit ObjectBase(ObjectType type) noexcept : type_{type} {}
  virtual ~ObjectBase() noexcept = default;

  std::string Inspect() const { return InspectImpl(); }
  ObjectType Type() const noexcept { return type_; }
  bool Ok() const noexcept { return type_ != ObjectType::kInvalid; }
  const ObjectBase* Ptr() const noexcept { return this; }

  virtual std::string InspectImpl() const { return {}; }

 private:
  ObjectType type_{ObjectType::kInvalid};
};

struct ObjectInterface {
  auto Inspect() const {
    return boost::te::call<std::string>(
        [](const auto& self) { return self.Inspect(); }, *this);
  }

  auto Type() const {
    return boost::te::call<ObjectType>(
        [](const auto& self) { return self.Type(); }, *this);
  }

  auto Ptr() const {
    return boost::te::call<const ObjectBase*>(
        [](const auto& self) { return self.Ptr(); }, *this);
  }

  auto Ok() const {
    return boost::te::call<bool>([](const auto& self) { return self.Ok(); },
                                 *this);
  }

  template <typename D>
  const D* PtrCast() const {
    static_assert(std::is_base_of_v<ObjectBase, D>,
                  "D is not dervied from ObjectBase");
    return static_cast<const D*>(Ptr());
  }
};

using Object = boost::te::poly<ObjectInterface>;

struct NullObject final : public ObjectBase {
  NullObject() : ObjectBase{ObjectType::kNull} {}
  std::string InspectImpl() const override { return "null"; }
};

struct IntObject final : public ObjectBase {
  using ValueType = int64_t;

  IntObject(ValueType value = 0) : ObjectBase{ObjectType::kInt}, value{value} {}
  std::string InspectImpl() const override { return std::to_string(value); }

  ValueType value{};
};

struct BoolObject final : public ObjectBase {
  using ValueType = bool;

  BoolObject(ValueType value = false)
      : ObjectBase{ObjectType::kBool}, value{value} {}
  std::string InspectImpl() const override { return value ? "true" : "false"; }

  ValueType value{};
};

struct ReturnObject final : public ObjectBase {
  ReturnObject(Object value = ObjectBase{})
      : ObjectBase{ObjectType::kReturn}, value{std::move(value)} {}
  std::string InspectImpl() const override { return value.Inspect(); }

  Object value{ObjectBase{}};
};

struct ErrorObject final : public ObjectBase {
  ErrorObject(std::string msg = {})
      : ObjectBase{ObjectType::kError}, msg{std::move(msg)} {}
  std::string InspectImpl() const override { return msg; }

  std::string msg;
};

}  // namespace monkey
