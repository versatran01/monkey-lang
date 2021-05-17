#pragma once

#include <stack>

#include "monkey/compiler.h"
#include "monkey/object.h"

namespace monkey {

class VirtualMachine {
 public:
  absl::Status Run(const Bytecode& bc);
  const Object& Top() const;

 private:
  Object Pop();

  std::stack<Object> stack;
};

}  // namespace monkey
