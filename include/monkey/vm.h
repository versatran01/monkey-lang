#pragma once

#include <stack>

#include "monkey/compiler.h"
#include "monkey/object.h"

namespace monkey {

class VirtualMachine {
 public:
  const Object& top() const { return stack.top(); }
  void Run(const Bytecode& bc);

 private:
  std::stack<Object> stack;
};

}  // namespace monkey
