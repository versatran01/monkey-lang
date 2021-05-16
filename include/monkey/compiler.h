#pragma once

#include <absl/status/statusor.h>

#include "monkey/ast.h"
#include "monkey/code.h"
#include "monkey/object.h"

namespace monkey {

struct Bytecode {
  Instruction inst;
  std::vector<Object> consts;
};

class Compiler {
 public:
  absl::StatusOr<Bytecode> Compile(const AstNode& node);

  absl::Status CompileImpl(const AstNode& node);

 private:
  int AddConstant(const Object& obj);
  int AddInstruction(const Instruction& ins);
  int Emit(Opcode op, const std::vector<int>& operands);

  Instruction ins_;
  std::vector<Object> consts_;
};

}  // namespace monkey
