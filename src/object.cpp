#include "monkey/object.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

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
    {ObjectType::kInt, "INT"},
    {ObjectType::kBool, "BOOL"},
    {ObjectType::kStr, "STR"},
    {ObjectType::kReturn, "RETURN"},
    {ObjectType::kError, "ERROR"},
    {ObjectType::kFunc, "FUNC"},
    {ObjectType::kBuiltin, "BUILTIN"},
    {ObjectType::kArray, "ARRAY"},
    {ObjectType::kDict, "DICT"},
    {ObjectType::kQuote, "QUOTE"},
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
      return absl::any_cast<BoolType>(value) ? "true" : "false";
    case ObjectType::kInt:
      return std::to_string(absl::any_cast<IntType>(value));
    case ObjectType::kStr:
      return absl::any_cast<StrType>(value);
    case ObjectType::kReturn:
      return Cast<Object>().Inspect();
    case ObjectType::kError:
      return absl::any_cast<std::string>(value);
    case ObjectType::kFunc:
      return Cast<FuncObject>().Inspect();
    case ObjectType::kBuiltin:
      return "Builtin Function";
    case ObjectType::kDict: {
      const auto pf = absl::PairFormatter(ObjFmt{}, ": ", ObjFmt{});
      return fmt::format("{{{}}}", absl::StrJoin(Cast<Dict>(), ", ", pf));
    }
    case ObjectType::kArray:
      return fmt::format("[{}]", absl::StrJoin(Cast<Array>(), ", ", ObjFmt{}));
    case ObjectType::kQuote:
      return fmt::format("QUOTE({})", Cast<ExprNode>().String());
    default:
      return fmt::format("Unknown type: {}", Type());
  }
}

std::ostream& operator<<(std::ostream& os, const Object& obj) {
  return os << fmt::format("Obj({}, {})", obj.Type(), obj.Inspect());
}

bool IsObjectHashable(ObjectType type) {
  return type == ObjectType::kBool || type == ObjectType::kInt ||
         type == ObjectType::kStr;
}

Object NullObj() { return Object{ObjectType::kNull}; }
Object IntObj(IntType value) { return {ObjectType::kInt, value}; }
Object StrObj(StrType value) { return {ObjectType::kStr, std::move(value)}; }
Object BoolObj(BoolType value) { return {ObjectType::kBool, value}; }
Object ErrorObj(const std::string& str) { return {ObjectType::kError, str}; }
Object ReturnObj(const Object& value) { return {ObjectType::kReturn, value}; }
Object FuncObj(const FuncObject& fn) { return {ObjectType::kFunc, fn}; }
Object ArrayObj(const Array& arr) { return {ObjectType::kArray, arr}; }
Object DictObj(const Dict& dict) { return {ObjectType::kDict, dict}; }
Object QuoteObj(const ExprNode& expr) { return {ObjectType::kQuote, expr}; }
Object BuiltinObj(const Builtin& fn) { return {ObjectType::kBuiltin, fn}; }

Object ToIntObj(const ExprNode& expr) {
  CHECK_EQ(expr.Type(), NodeType::kIntLiteral);
  return IntObj(expr.PtrCast<IntLiteral>()->value);
}

Object ToBoolObj(const ExprNode& expr) {
  CHECK_EQ(expr.Type(), NodeType::kBoolLiteral);
  return BoolObj(expr.PtrCast<BoolLiteral>()->value);
}

Object ToStrObj(const ExprNode& expr) {
  CHECK_EQ(expr.Type(), NodeType::kStrLiteral);
  return StrObj(expr.PtrCast<StrLiteral>()->value);
}

}  // namespace monkey
