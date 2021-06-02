#include "monkey/builtin.h"

#include <fmt/ostream.h>

#include <array>

namespace monkey {

namespace {

const std::string kWrongNumArgs = "wrong number of arguments";

const std::array<std::string, static_cast<size_t>(Builtin::kNumBuiltins)>
    gBuiltinStrs = {
        "len",
        "first",
        "last",
        "rest",
        "push",
        "puts",
};

Object BuiltinLen(absl::Span<const Object> args) {
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

Object BuiltinFirst(absl::Span<const Object> args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  if (arg.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("argument to `first` must be ARRAY, got {}", arg.Type()));
  }

  const auto& arr = arg.Cast<Array>();
  if (arr.empty()) return NullObj();
  return arr.front();
}

Object BuiltinLast(absl::Span<const Object> args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  if (arg.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("argument to `last` must be ARRAY, got {}", arg.Type()));
  }

  const auto& arr = arg.Cast<Array>();
  if (arr.empty()) return NullObj();
  return arr.back();
}

Object BuiltinRest(absl::Span<const Object> args) {
  if (args.size() != 1) {
    return ErrorObj(
        fmt::format("{}. got={}, want=1", kWrongNumArgs, args.size()));
  }

  const auto& arg = args.front();
  if (arg.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("argument to `rest` must be ARRAY, got {}", arg.Type()));
  }

  const auto& arr = arg.Cast<Array>();
  if (arr.empty()) return NullObj();

  //  std::vector<Object> rest{arr.begin() + 1, arr.end()};
  return ArrayObj({arr.begin() + 1, arr.end()});
}

// Object BuiltinPush(const std::vector<Object>& args) {
Object BuiltinPush(absl::Span<const Object> args) {
  if (args.size() != 2) {
    return ErrorObj(
        fmt::format("{}. got={}, want=2", kWrongNumArgs, args.size()));
  }

  const auto& arg0 = args.front();
  if (arg0.Type() != ObjectType::kArray) {
    return ErrorObj(
        fmt::format("argument to `push` must be ARRAY, got {}", arg0.Type()));
  }

  const auto& arr = arg0.Cast<Array>();
  std::vector<Object> copy{arr};
  copy.push_back(args[1]);
  return ArrayObj(std::move(copy));
}

Object BuiltinPuts(absl::Span<const Object> args) {
  for (const auto& arg : args) {
    fmt::print("{}\n", arg.Inspect());
  }
  return NullObj();
}

}  // namespace

std::vector<Object> MakeBuiltins() {
  std::vector<Object> v(static_cast<size_t>(Builtin::kNumBuiltins));
  v[static_cast<size_t>(Builtin::kLen)] = BuiltinObj({"len", BuiltinLen});
  v[static_cast<size_t>(Builtin::kFirst)] = BuiltinObj({"first", BuiltinFirst});
  v[static_cast<size_t>(Builtin::kLast)] = BuiltinObj({"last", BuiltinLast});
  v[static_cast<size_t>(Builtin::kRest)] = BuiltinObj({"rest", BuiltinRest});
  v[static_cast<size_t>(Builtin::kPush)] = BuiltinObj({"push", BuiltinPush});
  v[static_cast<size_t>(Builtin::kPuts)] = BuiltinObj({"puts", BuiltinPuts});
  return v;
}

std::string Repr(Builtin bt) {
  return gBuiltinStrs.at(static_cast<size_t>(bt));
}

std::ostream& operator<<(std::ostream& os, Builtin bt) {
  return os << Repr(bt);
}

const std::vector<Object>& GetBuiltins() {
  static std::vector<Object> builtins = MakeBuiltins();
  return builtins;
}

}  // namespace monkey
