#pragma once

#include <absl/container/flat_hash_map.h>

#include "monkey/object.h"

namespace monkey {

// Builtins
using BuiltinMap = absl::flat_hash_map<std::string, Object>;
BuiltinMap MakeBuiltins();

}  // namespace monkey
