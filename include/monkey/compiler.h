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

struct Emitted {
  Opcode op;
  size_t pos{};
};

struct Bytecode {
  Instruction ins;
  std::vector<Object> consts;
};

class Compiler {
 public:
  absl::StatusOr<Bytecode> Compile(const Program& program);

  const auto& timers() const noexcept { return timers_; }

 private:
  void Reset();
  absl::Status CompileImpl(const AstNode& node);

  /// Returns the index of the added object
  size_t AddConstant(const Object& obj);
  /// Returns the index of the added instruction
  size_t AddInstruction(const Instruction& ins);
  /// Returns the index of the added instruction
  size_t Emit(Opcode op, const std::vector<int>& operands = {});

  void SetEmitted(Opcode op, size_t pos);
  void RemoveLastOp(Opcode expected);
  void ReplaceInstruction(size_t pos, const Instruction& ins);
  void ChangeOperand(size_t pos, int operand);

  /// Compile expression
  absl::Status CompileIfExpr(const ExprNode& expr);
  absl::Status CompileInfixExpr(const ExprNode& expr);
  absl::Status CompilePrefixExpr(const ExprNode& expr);
  /// Compile statment
  absl::Status CompileExprStmt(const StmtNode& stmt);
  absl::Status CompileBlockStmt(const StmtNode& stmt);

  Instruction ins_;
  std::vector<Object> consts_;
  Emitted curr_, prev_;

  mutable TimerManager timers_;
};

}  // namespace monkey
