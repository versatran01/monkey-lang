#pragma once

#include <absl/container/flat_hash_map.h>

#include <stack>

#include "monkey/compiler.h"
#include "monkey/object.h"

namespace monkey {

struct Frame {
  const Instruction& Ins() const noexcept { return func.ins; }

  CompiledFunc func;
  mutable size_t ip{0};
};

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

  // Returns the stored last object on stack
  const Object& PopStack();
  void PushStack(Object obj);
  void ReplaceStackTop(Object obj);

  Frame PopFrame();
  void PushFrame(Frame frame);

  const auto& CurrFrame() const { return frames_.top(); }

  Object last_;
  std::stack<Object> stack_;
  std::stack<Frame> frames_;
  absl::flat_hash_map<int, Object> globals_;
};

}  // namespace monkey
