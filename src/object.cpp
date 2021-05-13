#include "monkey/object.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/ostream.h>

namespace monkey {

namespace {

struct ObjFmt {
  void operator()(std::string* out, const Object& obj) const {
    out->append(obj.Inspect());
  }
};

const auto gObjectTypeStrings = absl::flat_hash_map<ObjectType, std::string>{
    {ObjectType::kInvalid, "INVALID"},
    {ObjectType::kNull, "NULL"},
    {ObjectType::kInt, "INTEGER"},
    {ObjectType::kBool, "BOOLEAN"},
    {ObjectType::kStr, "STRING"},
    {ObjectType::kReturn, "RETURN"},
    {ObjectType::kError, "ERROR"},
    {ObjectType::kFunc, "FUNCTION"},
    {ObjectType::kBuiltinFunc, "BUILTIN_FUNC"},
    {ObjectType::kArray, "ARRAY"},
    {ObjectType::kDict, "DICT"},
};

}  // namespace

std::ostream& operator<<(std::ostream& os, ObjectType type) {
  return os << gObjectTypeStrings.at(type);
}

std::string FuncObject::Inspect() const {
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
    case ObjectType::kStr:
      return absl::any_cast<std::string>(value);
    case ObjectType::kReturn:
      return Cast<Object>().Inspect();
    case ObjectType::kError:
      return absl::any_cast<std::string>(value);
    case ObjectType::kFunc:
      return Cast<FuncObject>().Inspect();
    case ObjectType::kBuiltinFunc:
      return "Builtin Function";
    case ObjectType::kDict: {
      const auto pf = absl::PairFormatter(ObjFmt{}, ": ", ObjFmt{});
      return fmt::format("{{{}}}", absl::StrJoin(Cast<Dict>(), ", ", pf));
    }
    case ObjectType::kArray:
      return fmt::format("[{}]", absl::StrJoin(Cast<Array>(), ", ", ObjFmt{}));
    default:
      return fmt::format("Unknown type: {}", Type());
  }
}

std::ostream& operator<<(std::ostream& os, const Object& obj) {
  return os << fmt::format("Object({}, {})", obj.Type(), obj.Inspect());
}

Object NullObj() { return Object{ObjectType::kNull}; }
Object IntObj(int64_t value) { return {ObjectType::kInt, value}; }
Object StrObj(std::string value) {
  return {ObjectType::kStr, std::move(value)};
}
Object BoolObj(bool value) { return {ObjectType::kBool, value}; }
Object ErrorObj(std::string value) {
  return {ObjectType::kError, std::move(value)};
}
Object ReturnObj(Object value) {
  return {ObjectType::kReturn, std::move(value)};
}
Object FuncObj(FuncObject value) {
  return {ObjectType::kFunc, std::move(value)};
}
Object BuiltinFuncObj(BuiltinFunc value) {
  return {ObjectType::kBuiltinFunc, std::move(value)};
}
Object ArrayObj(Array value) { return {ObjectType::kArray, std::move(value)}; }
Object DictObject(Dict value) { return {ObjectType::kDict, std::move(value)}; }

}  // namespace monkey
