#pragma once

#include <absl/container/flat_hash_map.h>

#include "monkey/object.h"

namespace monkey {

enum class Builtin {
  kLen,
  kFirst,
  kLast,
  kRest,
  kPush,
  kPuts,
  kNumBuiltins,
};

std::string Repr(Builtin bt);
std::ostream& operator<<(std::ostream& os, Builtin bt);

// Builtins
const std::vector<Object>& GetBuiltins();

}  // namespace monkey
