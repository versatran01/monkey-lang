#include "monkey/compiler.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

static constexpr int kPlaceHolder = 0;

absl::StatusOr<Bytecode> Compiler::Compile(const Program& program) {
  auto _ = timers_.Scoped("CompileProgram");

  for (const auto& stmt : program.statements) {
    auto status = CompileImpl(stmt);
    if (!status.ok()) return status;
  }

  Bytecode bc{std::move(ins_), std::move(consts_)};
  Reset();
  return bc;
}

void Compiler::Reset() {
  ins_ = Instruction{};
  consts_.clear();
}

absl::Status Compiler::CompileImpl(const AstNode& node) {
  CHECK_NE(node.Type(), NodeType::kProgram);

  switch (node.Type()) {
    case NodeType::kBlockStmt: {
      return CompileBlockStmt(node);
    }
    case NodeType::kExprStmt: {
      return CompileExprStmt(node);
    }
    case NodeType::kLetStmt: {
      return CompileLetStmt(node);
    }
    case NodeType::kIfExpr: {
      return CompileIfExpr(node);
    }
    case NodeType::kPrefixExpr: {
      return CompilePrefixExpr(node);
    }
    case NodeType::kInfixExpr: {
      return CompileInfixExpr(node);
    }
    case NodeType::kIntLiteral: {
      const auto obj = ToIntObj(node);
      Emit(Opcode::kConst, {static_cast<int>(AddConstant(obj))});
      break;
    }
    case NodeType::kBoolLiteral: {
      const auto* ptr = node.PtrCast<BoolLiteral>();
      if (ptr->value) {
        Emit(Opcode::kTrue);
      } else {
        Emit(Opcode::kFalse);
      }
      break;
    }
    default:
      return MakeError("Internal Compiler Error: Unhandled ast node: " +
                       Repr(node.Type()));
  }

  return absl::OkStatus();
}

size_t Compiler::AddConstant(const Object& obj) {
  consts_.push_back(obj);
  return consts_.size() - 1;
}

size_t Compiler::AddInstruction(const Instruction& ins) {
  const auto pos = ins_.NumBytes();
  ins_.Append(ins);
  return pos;
}

size_t Compiler::Emit(Opcode op, const std::vector<int>& operands) {
  const auto pos = AddInstruction(Encode(op, operands));
  SetEmitted(op, pos);
  return pos;
}

void Compiler::SetEmitted(Opcode op, size_t pos) {
  prev_ = curr_;
  curr_.op = op;
  curr_.pos = pos;
}

void Compiler::RemoveLastOp(Opcode expected) {
  auto op = ToOpcode(ins_.PopBack());
  CHECK_EQ(op, expected);
  curr_ = prev_;
}

void Compiler::ReplaceInstruction(size_t pos, const Instruction& ins) {
  CHECK_LE(pos + ins.NumBytes(), ins_.NumBytes());
  // Also check the opcode are the same
  CHECK_EQ(ToOpcode(ins_.bytes[pos]), ToOpcode(ins.bytes[0]))
      << "Replacing instruction with different types is not allowed";
  for (size_t i = 0; i < ins.NumBytes(); ++i) {
    ins_.bytes[pos + i] = ins.bytes[i];
  }
}

void Compiler::ChangeOperand(size_t pos, int operand) {
  const auto op = ToOpcode(ins_.bytes.at(pos));
  const auto new_ins = Encode(op, {operand});
  ReplaceInstruction(pos, new_ins);
}

absl::Status Compiler::CompileIfExpr(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<IfExpr>();
  CHECK_NOTNULL(ptr);

  // Compile condition
  auto status = CompileImpl(ptr->cond);
  if (!status.ok()) return status;

  // Emit an `OpJumpNotTruthy` with a bogus value
  const auto jnt_pos = Emit(Opcode::kJumpNotTrue, {kPlaceHolder});

  // Compile true block
  status.Update(CompileImpl(ptr->true_block));
  if (!status.ok()) return status;

  if (curr_.op == Opcode::kPop) RemoveLastOp(Opcode::kPop);

  // Emit an `OpJump` with a bogus value
  const auto jmp_pos = Emit(Opcode::kJump, {kPlaceHolder});
  ChangeOperand(jnt_pos, ins_.NumBytes());

  if (ptr->false_block.empty()) {
    Emit(Opcode::kNull);
  } else {
    status.Update(CompileImpl(ptr->false_block));
    if (!status.ok()) return status;

    if (curr_.op == Opcode::kPop) RemoveLastOp(Opcode::kPop);
  }

  ChangeOperand(jmp_pos, ins_.NumBytes());
  return absl::OkStatus();
}

absl::Status Compiler::CompileInfixExpr(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<InfixExpr>();
  CHECK_NOTNULL(ptr);

  // Reorder < to >, by creating a new expr
  InfixExpr infix;
  if (ptr->op == "<") {
    infix.op = ">";
    infix.lhs = ptr->rhs;
    infix.rhs = ptr->lhs;
    ptr = &infix;
  }

  auto status = CompileImpl(ptr->lhs);
  if (!status.ok()) return status;

  status.Update(CompileImpl(ptr->rhs));
  if (!status.ok()) return status;

  if (ptr->op == "+") {
    Emit(Opcode::kAdd);
  } else if (ptr->op == "-") {
    Emit(Opcode::kSub);
  } else if (ptr->op == "*") {
    Emit(Opcode::kMul);
  } else if (ptr->op == "/") {
    Emit(Opcode::kDiv);
  } else if (ptr->op == ">") {
    Emit(Opcode::kGt);
  } else if (ptr->op == "==") {
    Emit(Opcode::kEq);
  } else if (ptr->op == "!=") {
    Emit(Opcode::kNe);
  } else {
    return MakeError("Unknown operator " + ptr->op);
  }

  return absl::OkStatus();
}

absl::Status Compiler::CompilePrefixExpr(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<PrefixExpr>();
  CHECK_NOTNULL(ptr);

  auto status = CompileImpl(ptr->rhs);
  if (!status.ok()) return status;

  if (ptr->op == "!") {
    Emit(Opcode::kBang);
  } else if (ptr->op == "-") {
    Emit(Opcode::kMinus);
  } else {
    return MakeError("Unknown operator: " + ptr->op);
  }

  return absl::OkStatus();
}

absl::Status Compiler::CompileLetStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<LetStmt>();
  CHECK_NOTNULL(ptr);
  auto status = CompileImpl(ptr->expr);
  if (!status.ok()) return status;
  // Add to symbol table
  const auto& symbol = stable_.Define(ptr->name.value);
  Emit(Opcode::kSetGlobal, {symbol.index});
  return absl::OkStatus();
}

absl::Status Compiler::CompileExprStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<ExprStmt>();
  CHECK_NOTNULL(ptr);

  auto status = CompileImpl(ptr->expr);
  if (!status.ok()) return status;

  Emit(Opcode::kPop);
  return absl::OkStatus();
}

absl::Status Compiler::CompileBlockStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<BlockStmt>();
  CHECK_NOTNULL(ptr);

  for (const auto& stmt : ptr->statements) {
    auto status = CompileImpl(stmt);
    if (!status.ok()) return status;
  }

  return absl::OkStatus();
}

std::string Symbol::Repr() const {
  return fmt::format("Symbol(name={}, scope={}, ind={})", name, scope, index);
}

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
  return os << symbol.Repr();
}

Symbol& SymbolTable::Define(const std::string& name) {
  return store_[name] = {name, kGlobalScope, num_defs_++};
}

absl::optional<Symbol> SymbolTable::Resolve(const std::string& name) const {
  const auto it = store_.find(name);
  if (it == store_.end()) return absl::nullopt;
  return it->second;
}

}  // namespace monkey
