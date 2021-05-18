#pragma once

#include <stack>

#include "monkey/compiler.h"
#include "monkey/object.h"

namespace monkey {

class VirtualMachine {
 public:
  absl::Status Run(const Bytecode& bc);
  const Object& Top() const;

  const Object& last() const { return last_; }

 private:
  absl::Status ExecBinaryOp(Opcode op);
  absl::Status ExecIntBinaryOp(const Object& lhs, Opcode op, const Object& rhs);
  absl::Status ExecComparison(Opcode op);
  absl::Status ExecIntComparison(const Object& lhs,
                                 Opcode op,
                                 const Object& rhs);

  Object Pop();
  void Push(Object obj);

  std::stack<Object> stack;
  Object last_{NullObj()};
};

}  // namespace monkey
