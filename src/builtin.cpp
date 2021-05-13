#include "monkey/builtin.h"

#include <fmt/ostream.h>

namespace monkey {

namespace {

const std::string kWrongNumArgs = "wrong number of arguments";

Object BuiltinLen(const std::vector<Object>& args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  switch (arg.Type()) {
    case ObjectType::kStr:
      return IntObj(static_cast<int64_t>(arg.Cast<std::string>().size()));
    case ObjectType::kArray:
      return IntObj(static_cast<int64_t>(arg.Cast<Array>().size()));
    default:
      return ErrorObj(
          fmt::format("argument to `len` not supported, got {}", arg.Type()));
  }
}

Object BuiltinFirst(const std::vector<Object>& args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  if (arg.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("arguent to `first` must be ARRAY, got {}", arg.Type()));
  }

  const auto& arr = arg.Cast<Array>();
  if (!arr.empty()) {
    return arr.front();
  }
  return NullObj();
}

}  // namespace

BuiltinMap MakeBuiltins() {
  BuiltinMap map;
  map["len"] = BuiltinFuncObj(BuiltinLen);
  map["first"] = BuiltinFuncObj(BuiltinFirst);
  return map;
}

}  // namespace monkey
