#pragma once

#include <absl/container/flat_hash_map.h>

#include <stack>

#include "monkey/compiler.h"
#include "monkey/frame.h"
#include "monkey/object.h"

namespace monkey {

class VirtualMachine {
 public:
  absl::Status Run(const Bytecode& bc);
  const Object& Top() const;
  const Object& Last() const { return last_; }

 private:
  absl::Status ExecBinaryOp(Opcode op);
  absl::Status ExecIntBinaryOp(const Object& lhs, Opcode op, const Object& rhs);
  absl::Status ExecStrBinaryOp(const Object& lhs, Opcode op, const Object& rhs);

  absl::Status ExecBangOp();
  absl::Status ExecMinusOp();

  absl::Status ExecComparison(Opcode op);
  absl::Status ExecIntComp(const Object& lhs, Opcode op, const Object& rhs);

  absl::Status ExecIndexExpr(const Object& lhs, const Object& index);
  absl::Status ExecDictIndex(const Object& lhs, const Object& index);
  absl::Status ExecArrayIndex(const Object& lhs, const Object& index);

  Object BuildArray(size_t size);
  Object BuildDict(size_t size);

  Object PopStack();
  void PushStack(Object obj);

  void PushFrame(const Frame& frame);

  Object last_;
  std::stack<Object> stack_;
  std::stack<Frame> frames_;
  absl::flat_hash_map<int, Object> globals_;
};

}  // namespace monkey
