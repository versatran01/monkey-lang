#pragma once

#include <deque>
#include <stack>

#include "monkey/compiler.h"
#include "monkey/object.h"

namespace monkey {

class VirtualMachine {
 public:
  absl::Status Run(const Bytecode& bc);
  const Object& Top() const;
  const Object& Last() const;

 private:
  absl::Status ExecBinaryOp(Opcode op);
  absl::Status ExecIntBinaryOp(const Object& lhs, Opcode op, const Object& rhs);
  absl::Status ExecComparison(Opcode op);
  absl::Status ExecIntComparison(const Object& lhs,
                                 Opcode op,
                                 const Object& rhs);
  absl::Status ExecBangOp();
  absl::Status ExecMinusOp();

  Object Pop();
  void Push(Object obj);

  size_t sp{0};
  std::deque<Object> stack;
};

}  // namespace monkey
