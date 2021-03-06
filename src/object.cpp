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
    {ObjectType::kArray, "ARRAY"},
    {ObjectType::kDict, "DICT"},
    {ObjectType::kQuote, "QUOTE"},
    {ObjectType::kCompiled, "COMPILED"},
    {ObjectType::kBuiltinFunc, "BUILTIN_FUNC"},
    {ObjectType::kClosure, "CLOSURE"},
};

}  // namespace

std::string Repr(ObjectType type) { return gObjectTypeStrings.at(type); }

std::ostream& operator<<(std::ostream& os, ObjectType type) {
  return os << Repr(type);
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

std::string CompiledFunc::Inspect() const {
  //  return fmt::format("{}", fmt::ptr(this));
  return ins.Repr();
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
    case ObjectType::kBuiltinFunc:
      return Cast<BuiltinFunc>().name + "()";
    case ObjectType::kDict: {
      const auto pf = absl::PairFormatter(ObjFmt{}, ": ", ObjFmt{});
      return fmt::format("{{{}}}", absl::StrJoin(Cast<Dict>(), ", ", pf));
    }
    case ObjectType::kArray:
      return fmt::format("[{}]", absl::StrJoin(Cast<Array>(), ", ", ObjFmt{}));
    case ObjectType::kQuote:
      return Cast<ExprNode>().String();
    case ObjectType::kCompiled:
      return Cast<CompiledFunc>().Inspect();
    case ObjectType::kClosure:
      return Cast<Closure>().Inspect();
    default:
      return fmt::format("Unknown type: {}", Type());
  }
}

std::ostream& operator<<(std::ostream& os, const Object& obj) {
  return os << fmt::format("{}({})", obj.Type(), obj.Inspect());
}

Object NullObj() { return Object{ObjectType::kNull}; }
Object IntObj(IntType value) { return {ObjectType::kInt, value}; }
Object StrObj(StrType value) { return {ObjectType::kStr, std::move(value)}; }
Object BoolObj(BoolType value) { return {ObjectType::kBool, value}; }
Object ErrorObj(StrType str) { return {ObjectType::kError, std::move(str)}; }
Object ReturnObj(Object obj) { return {ObjectType::kReturn, std::move(obj)}; }
Object FuncObj(const FuncObject& fn) { return {ObjectType::kFunc, fn}; }
Object ArrayObj(Array arr) { return {ObjectType::kArray, std::move(arr)}; }
Object DictObj(Dict dict) { return {ObjectType::kDict, std::move(dict)}; }
Object QuoteObj(const ExprNode& expr) { return {ObjectType::kQuote, expr}; }
Object BuiltinObj(BuiltinFunc fn) {
  return {ObjectType::kBuiltinFunc, std::move(fn)};
}
Object CompiledObj(CompiledFunc fn) {
  return {ObjectType::kCompiled, std::move(fn)};
}
Object CompiledObj(const std::vector<Instruction>& ins) {
  return {ObjectType::kCompiled, CompiledFunc{ConcatInstructions(ins)}};
}
Object ClosureObj(Closure cl) { return {ObjectType::kClosure, std::move(cl)}; }

Object ToIntObj(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<IntLiteral>();
  CHECK_NOTNULL(ptr);
  return IntObj(ptr->value);
}

Object ToBoolObj(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<BoolLiteral>();
  CHECK_NOTNULL(ptr);
  return BoolObj(ptr->value);
}

Object ToStrObj(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<StrLiteral>();
  CHECK_NOTNULL(ptr);
  return StrObj(ptr->value);
}

bool IsObjTruthy(const Object& obj) {
  switch (obj.Type()) {
    case ObjectType::kNull:
      return false;
    case ObjectType::kBool:
      return obj.Cast<BoolType>();
    default:
      return true;
  }
}

bool IsObjError(const Object& obj) noexcept {
  return obj.Type() == ObjectType::kError;
}

bool IsObjHashable(const Object& obj) noexcept {
  auto type = obj.Type();
  return type == ObjectType::kBool || type == ObjectType::kInt ||
         type == ObjectType::kStr;
}

bool operator==(const Object& lhs, const Object& rhs) {
  // If not same type return false
  if (lhs.Type() != rhs.Type()) return false;
  // All nulls are the same
  if (lhs.Type() == ObjectType::kNull) return true;
  if (lhs.Type() == ObjectType::kBool) {
    return lhs.Cast<BoolType>() == rhs.Cast<BoolType>();
  }
  if (lhs.Type() == ObjectType::kInt) {
    return lhs.Cast<IntType>() == rhs.Cast<IntType>();
  }
  return lhs.Inspect() == rhs.Inspect();
}

}  // namespace monkey
