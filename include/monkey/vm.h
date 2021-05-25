#pragma once

#include <absl/container/flat_hash_map.h>

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
  absl::Status ExecStrBinaryOp(const Object& lhs, Opcode op, const Object& rhs);

  absl::Status ExecBangOp();
  absl::Status ExecMinusOp();

  absl::Status ExecComparison(Opcode op);
  absl::Status ExecIntComp(const Object& lhs, Opcode op, const Object& rhs);

  Object Pop();
  void Push(Object obj);

  size_t sp_{0};
  std::deque<Object> stack_;
  absl::flat_hash_map<int, Object> globals_;
};

}  // namespace monkey
