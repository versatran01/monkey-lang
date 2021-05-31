#pragma once

#include <absl/container/flat_hash_map.h>

#include <stack>

#include "monkey/compiler.h"
#include "monkey/object.h"

namespace monkey {

struct Frame {
  const Instruction& Ins() const noexcept { return func.ins; }

  CompiledFunc func;
  size_t bp{0};  // base pointer
  size_t ip{0};  // instruction pointer
};

class VirtualMachine {
 public:
  absl::Status Run(const Bytecode& bc);
  const Object& StackTop() const;
  const Object& Last() const;

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
  Object PopStack();
  void PushStack(Object obj);
  void ReplaceStackTop(Object obj);

  Frame PopFrame();
  void PushFrame(Frame frame);

  const Frame& CurrFrame() const { return frames_.top(); }
  Frame& CurrFrame() { return frames_.top(); }

  void AllocateLocal(size_t num_locals);

  size_t sp_{0};  // sp -> last, sp-1 -> top
  Object last_;
  std::deque<Object> stack_;
  std::stack<Frame> frames_;
  absl::flat_hash_map<int, Object> globals_;
};

}  // namespace monkey
