#pragma once

#include <absl/status/statusor.h>

#include "monkey/ast.h"
#include "monkey/code.h"
#include "monkey/instruction.h"
#include "monkey/object.h"
#include "monkey/symbol.h"
#include "monkey/timer.h"

namespace monkey {

inline absl::Status MakeError(absl::string_view msg) {
  return absl::InternalError(msg);
}

inline static const absl::Status kOkStatus = absl::OkStatus();

struct Bytecode {
  Instruction ins;
  std::vector<Object> consts;
};

class Compiler {
 public:
  Compiler();

  absl::StatusOr<Bytecode> Compile(const Program& program);

  const auto& timers() const noexcept { return timers_; }

  // Emitted opcode and position in instruction
  struct Emitted {
    Opcode op;
    size_t pos{};
  };

  struct Scope {
    Instruction ins;
    Emitted last;
    Emitted prev;
  };

  Instruction& ScopedIns() { return CurrScope().ins; }
  const Instruction& ScopedIns() const { return CurrScope().ins; }

  Emitted& ScopedLast() { return CurrScope().last; }
  const Emitted& ScopedLast() const { return CurrScope().last; }

  const Emitted& ScopedPrev() const { return CurrScope().prev; }

  /// Scope related
  void EnterScope();
  Instruction ExitScope();
  Scope& CurrScope() { return scopes_.back(); }
  const Scope& CurrScope() const { return scopes_.back(); }
  size_t NumScopes() const noexcept { return scopes_.size(); }

  /// Returns the index of the added instruction
  size_t Emit(Opcode op, const std::vector<int>& operands = {});
  size_t Emit(Opcode op, int operand);

 private:
  void Reset();
  absl::Status CompileImpl(const AstNode& node);

  /// Returns the index of the added object
  size_t AddConstant(const Object& obj);
  /// Returns the index of the added instruction
  size_t AddInstruction(const Instruction& ins);

  void SetEmitted(Opcode op, size_t pos);
  void RemoveLastOp(Opcode expected);
  void ReplaceInstruction(size_t pos, const Instruction& ins);
  void ChangeOperand(size_t pos, int operand);

  /// Compile expression
  absl::Status CompileIfExpr(const ExprNode& expr);
  absl::Status CompileCallExpr(const ExprNode& expr);
  absl::Status CompileIndexExpr(const ExprNode& expr);
  absl::Status CompileInfixExpr(const ExprNode& expr);
  absl::Status CompilePrefixExpr(const ExprNode& expr);
  absl::Status CompileIdentifier(const ExprNode& expr);
  /// Compile statement
  absl::Status CompileLetStmt(const StmtNode& stmt);
  absl::Status CompileExprStmt(const StmtNode& stmt);
  absl::Status CompileBlockStmt(const StmtNode& stmt);
  absl::Status CompileReturnStmt(const StmtNode& stmt);

  std::vector<Scope> scopes_;
  SymbolTable stable_;
  std::vector<Object> consts_;
  mutable TimerManager timers_;
};

}  // namespace monkey
