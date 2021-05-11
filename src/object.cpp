#include "monkey/object.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/ostream.h>

namespace monkey {

namespace {

const auto gObjectTypeStrings = absl::flat_hash_map<ObjectType, std::string>{
    {ObjectType::kInvalid, "INVALID"},
    {ObjectType::kNull, "NULL"},
    {ObjectType::kInt, "INTEGER"},
    {ObjectType::kBool, "BOOLEAN"},
    {ObjectType::kReturn, "RETURN"},
    {ObjectType::kError, "ERROR"},
    {ObjectType::kFunction, "FUNCTION"},
};

}  // namespace

std::ostream& operator<<(std::ostream& os, ObjectType type) {
  return os << gObjectTypeStrings.at(type);
}

std::string FnObject::Inspect() const {
  return fmt::format(
      "fn({}) {{\n{}\n}}",
      absl::StrJoin(params,
                    ", ",
                    [](std::string* out, const Identifier& ident) {
                      out->append(ident.value);
                    }),
      body.String());
}

std::string Object::Inspect() const {
  switch (Type()) {
    case ObjectType::kNull:
      return "Null";
    case ObjectType::kBool:
      return absl::any_cast<bool>(value) ? "true" : "false";
    case ObjectType::kInt:
      return std::to_string(absl::any_cast<int64_t>(value));
    case ObjectType::kReturn:
      return Cast<Object>().Inspect();
    case ObjectType::kError:
      return absl::any_cast<std::string>(value);
    case ObjectType::kFunction:
      return Cast<FnObject>().Inspect();
    default:
      return fmt::format("Unknown type: {}", Type());
  }
}

std::ostream& operator<<(std::ostream& os, const Object& obj) {
  return os << fmt::format("Object({}, {})", obj.Type(), obj.Inspect());
}

Object NullObject() { return Object{ObjectType::kNull}; }
Object IntObject(int64_t value) { return {ObjectType::kInt, value}; }
Object BoolObject(bool value) { return {ObjectType::kBool, value}; }
Object ErrorObject(std::string value) {
  return {ObjectType::kError, std::move(value)};
}
Object FunctionObject(FnObject value) {
  return {ObjectType::kFunction, std::move(value)};
}

}  // namespace monkey
