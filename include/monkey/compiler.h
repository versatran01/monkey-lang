#pragma once

#include <absl/status/statusor.h>

#include "monkey/ast.h"
#include "monkey/code.h"
#include "monkey/object.h"
#include "monkey/timer.h"

namespace monkey {

inline absl::Status MakeError(absl::string_view msg) {
  return absl::InternalError(msg);
}

struct Bytecode {
  Instruction ins;
  std::vector<Object> consts;
};

class Compiler {
 public:
  absl::StatusOr<Bytecode> Compile(const Program& program);

  const auto& timers() const noexcept { return timers_; }

 private:
  absl::Status CompileImpl(const AstNode& node);

  int AddConstant(const Object& obj);
  int AddInstruction(const Instruction& ins);
  int Emit(Opcode op, const std::vector<int>& operands = {});

  Instruction ins_;
  std::vector<Object> consts_;

  mutable TimerManager timers_;
};

}  // namespace monkey
