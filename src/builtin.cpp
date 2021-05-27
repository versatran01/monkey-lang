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
      return IntObj(static_cast<IntType>(arg.Cast<std::string>().size()));
    case ObjectType::kArray:
      return IntObj(static_cast<IntType>(arg.Cast<Array>().size()));
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
  if (arr.empty()) {
    return NullObj();
  }
  return arr.front();
}

Object BuiltinLast(const std::vector<Object>& args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  if (arg.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("arguent to `last` must be ARRAY, got {}", arg.Type()));
  }

  const auto& arr = arg.Cast<Array>();
  if (arr.empty()) {
    return NullObj();
  }
  return arr.back();
}

Object BuiltinRest(const std::vector<Object>& args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  if (arg.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("arguent to `rest` must be ARRAY, got {}", arg.Type()));
  }

  const auto& arr = arg.Cast<Array>();
  if (arr.empty()) {
    return NullObj();
  }

  //  std::vector<Object> rest{arr.begin() + 1, arr.end()};
  return ArrayObj({arr.begin() + 1, arr.end()});
}

Object BuiltinPush(const std::vector<Object>& args) {
  if (args.size() != 2) {
    return ErrorObj(
        fmt::format("{}. got={}, want=2", kWrongNumArgs, args.size()));
  }

  const auto& arg0 = args.front();
  if (arg0.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("arguent to `push` must be ARRAY, got {}", arg0.Type()));
  }

  const auto& arr = arg0.Cast<Array>();
  std::vector<Object> copy{arr};
  copy.push_back(args[1]);
  return ArrayObj(std::move(copy));
}

Object BuiltinPuts(const std::vector<Object>& args) {
  for (const auto& arg : args) {
    fmt::print("{}\n", arg.Inspect());
  }
  return NullObj();
}

}  // namespace

BuiltinMap MakeBuiltins() {
  BuiltinMap map;
  map["len"] = BuiltinObj({"len", BuiltinLen});
  map["first"] = BuiltinObj({"first", BuiltinFirst});
  map["last"] = BuiltinObj({"last", BuiltinLast});
  map["rest"] = BuiltinObj({"rest", BuiltinRest});
  map["push"] = BuiltinObj({"push", BuiltinPush});
  map["puts"] = BuiltinObj({"puts", BuiltinPuts});
  return map;
}

}  // namespace monkey
