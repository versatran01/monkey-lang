#pragma once

#include <absl/status/statusor.h>

#include "monkey/ast.h"
#include "monkey/code.h"
#include "monkey/instruction.h"
#include "monkey/object.h"
#include "monkey/timer.h"

namespace monkey {

inline absl::Status MakeError(absl::string_view msg) {
  return absl::InternalError(msg);
}

inline static const absl::Status kOkStatus = absl::OkStatus();

// Emitted opcode and position in instruction
struct Emitted {
  Opcode op;
  size_t pos{};
};

struct Bytecode {
  Instruction ins;
  std::vector<Object> consts;

  Byte ByteAt(size_t n) const { return ins.bytes.at(n); }
  const Byte* BytePtr(size_t n) const { return &ins.bytes.at(n); }
};

using SymbolScope = std::string;

static const SymbolScope kGlobalScope = "GLOBAL";
static const SymbolScope kLocalScope = "LOCAL";

struct Symbol {
  std::string name;
  SymbolScope scope;
  int index;

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);

  friend bool operator==(const Symbol& lhs, const Symbol& rhs) {
    return lhs.index == rhs.index && lhs.scope == rhs.scope &&
           lhs.name == rhs.name;
  }

  friend bool operator!=(const Symbol& lhs, const Symbol& rhs) {
    return !(lhs == rhs);
  }
};

using SymbolDict = absl::flat_hash_map<std::string, Symbol>;

class SymbolTable {
 public:
  Symbol& Define(const std::string& name);
  absl::optional<Symbol> Resolve(const std::string& name) const;

  int NumDefs() const { return num_defs_; }

 private:
  SymbolDict store_;
  int num_defs_{0};
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
  size_t Emit(Opcode op, int operand);

  void SetEmitted(Opcode op, size_t pos);
  void RemoveLastOp(Opcode expected);
  void ReplaceInstruction(size_t pos, const Instruction& ins);
  void ChangeOperand(size_t pos, int operand);

  /// Compile expression
  absl::Status CompileIfExpr(const ExprNode& expr);
  absl::Status CompileIndexExpr(const ExprNode& expr);
  absl::Status CompileInfixExpr(const ExprNode& expr);
  absl::Status CompilePrefixExpr(const ExprNode& expr);
  absl::Status CompileIdentifier(const ExprNode& expr);
  /// Compile statement
  absl::Status CompileLetStmt(const StmtNode& stmt);
  absl::Status CompileExprStmt(const StmtNode& stmt);
  absl::Status CompileBlockStmt(const StmtNode& stmt);

  Instruction ins_;
  Emitted curr_, prev_;
  SymbolTable stable_;
  std::vector<Object> consts_;
  mutable TimerManager timers_;
};

}  // namespace monkey
