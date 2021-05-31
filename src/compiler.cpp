#include "monkey/compiler.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

#include "monkey/builtin.h"

namespace monkey {

namespace {
static constexpr int kPlaceHolder = 0;
const auto gBuiltins = MakeBuiltins();
}  // namespace

Compiler::Compiler() {
  EnterScope();

  for (size_t i = 0; i < gBuiltins.size(); ++i) {
    CurrTable().DefineBuiltin(gBuiltins[i].Cast<BuiltinFunc>().name, i);
  }
}

absl::StatusOr<Bytecode> Compiler::Compile(const Program& program) {
  auto _ = timers_.Scoped("CompileProgram");

  for (const auto& stmt : program.statements) {
    auto status = CompileImpl(stmt);
    if (!status.ok()) return status;
  }

  // Maybe don't use move
  Bytecode bc{std::move(ScopedIns()), std::move(consts_)};
  Reset();
  return bc;
}

void Compiler::EnterScope() {
  // Add a new scope
  scopes_.push_back({});
  // Create a new enclosed local symbol table unless it's the first global one
  const auto* outer = tables_.empty() ? nullptr : tables_.back().get();
  tables_.push_back(std::make_unique<SymbolTable>(outer));
}

Instruction Compiler::ExitScope() {
  Instruction ins = std::move(ScopedIns());
  scopes_.pop_back();
  tables_.pop_back();
  return ins;
}

void Compiler::Reset() {
  scopes_.clear();
  consts_.clear();
  tables_.clear();
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
    case NodeType::kReturnStmt: {
      return CompileReturnStmt(node);
    }
    case NodeType::kIdentifier: {
      return CompileIdentifier(node);
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
    case NodeType::kIndexExpr: {
      return CompileIndexExpr(node);
    }
    case NodeType::kCallExpr: {
      return CompileCallExpr(node);
    }
    case NodeType::kIntLiteral: {
      Emit(Opcode::kConst, static_cast<int>(AddConstant(ToIntObj(node))));
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
    case NodeType::kStrLiteral: {
      Emit(Opcode::kConst, static_cast<int>(AddConstant(ToStrObj(node))));
      break;
    }
    case NodeType::kArrayLiteral: {
      const auto* ptr = node.PtrCast<ArrayLiteral>();
      for (const auto& elem : ptr->elements) {
        auto status = CompileImpl(elem);
        if (!status.ok()) return status;
      }

      Emit(Opcode::kArray, static_cast<int>(ptr->elements.size()));
      break;
    }
    case NodeType::kDictLiteral: {
      const auto* ptr = node.PtrCast<DictLiteral>();

      for (const auto& pair : ptr->pairs) {
        auto status = CompileImpl(pair.first);
        if (!status.ok()) return status;
        status.Update(CompileImpl(pair.second));
        if (!status.ok()) return status;
      }

      Emit(Opcode::kDict, static_cast<int>(ptr->pairs.size()) * 2);
      break;
    }
    case NodeType::kFuncLiteral: {
      return CompileFuncLiteral(node);
    }
    default:
      return MakeError("Internal Compiler Error: Unhandled ast node: " +
                       Repr(node.Type()));
  }

  return kOkStatus;
}

size_t Compiler::AddConstant(Object obj) {
  consts_.push_back(std::move(obj));
  return consts_.size() - 1;
}

size_t Compiler::AddInstruction(const Instruction& ins) {
  auto& curr_ins = ScopedIns();
  const auto pos = curr_ins.NumBytes();
  curr_ins.Append(ins);
  return pos;
}

size_t Compiler::Emit(Opcode op, const std::vector<int>& operands) {
  const auto pos = AddInstruction(Encode(op, operands));
  SetEmitted(op, pos);
  return pos;
}

size_t Compiler::Emit(Opcode op, int operand) {
  const auto pos = AddInstruction(Encode(op, operand));
  SetEmitted(op, pos);
  return pos;
}

void Compiler::SetEmitted(Opcode op, size_t pos) {
  CurrScope().prev = CurrScope().last;
  CurrScope().last = {op, pos};
}

void Compiler::RemoveLastOp(Opcode expected) {
  auto& ins = ScopedIns();
  auto op = ToOpcode(ins.PopBack());
  CHECK_EQ(op, expected);
  CurrScope().last = CurrScope().prev;
}

void Compiler::ReplaceInstruction(size_t pos, const Instruction& ins) {
  auto& curr_ins = ScopedIns();
  CHECK_LE(pos + ins.NumBytes(), curr_ins.NumBytes());
  // Also check the opcode are the same
  //  CHECK_EQ(ToOpcode(curr_ins.bytes[pos]), ToOpcode(ins.bytes[0]))
  //      << "Replacing instruction with different types is not allowed";
  for (size_t i = 0; i < ins.NumBytes(); ++i) {
    curr_ins.bytes[pos + i] = ins.bytes[i];
  }
}

void Compiler::ChangeOperand(size_t pos, int operand) {
  const auto op = ToOpcode(ScopedIns().bytes.at(pos));
  const auto new_ins = Encode(op, operand);
  ReplaceInstruction(pos, new_ins);
}

absl::Status Compiler::CompileIfExpr(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<IfExpr>();
  CHECK_NOTNULL(ptr);

  // Compile condition
  auto status = CompileImpl(ptr->cond);
  if (!status.ok()) return status;

  // Emit an `OpJumpNotTruthy` with a bogus value
  const auto jnt_pos = Emit(Opcode::kJumpNotTrue, kPlaceHolder);

  // Compile true block
  status.Update(CompileImpl(ptr->true_block));
  if (!status.ok()) return status;

  if (ScopedLast().op == Opcode::kPop) RemoveLastOp(Opcode::kPop);

  // Emit an `OpJump` with a bogus value
  const auto jmp_pos = Emit(Opcode::kJump, kPlaceHolder);
  ChangeOperand(jnt_pos, static_cast<int>(ScopedIns().NumBytes()));

  if (ptr->false_block.empty()) {
    Emit(Opcode::kNull);
  } else {
    status.Update(CompileImpl(ptr->false_block));
    if (!status.ok()) return status;

    if (ScopedLast().op == Opcode::kPop) RemoveLastOp(Opcode::kPop);
  }

  ChangeOperand(jmp_pos, static_cast<int>(ScopedIns().NumBytes()));
  return kOkStatus;
}

absl::Status Compiler::CompileCallExpr(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<CallExpr>();
  CHECK_NOTNULL(ptr);
  auto status = kOkStatus;
  status.Update(CompileImpl(ptr->func));
  if (!status.ok()) return status;

  for (const auto& arg : ptr->args) {
    status.Update(CompileImpl(arg));
    if (!status.ok()) return status;
  }

  Emit(Opcode::kCall, static_cast<int>(ptr->args.size()));
  return status;
}

absl::Status Compiler::CompileIndexExpr(const ExprNode& expr) {
  const auto* ptr = expr.PtrCast<IndexExpr>();
  CHECK_NOTNULL(ptr);
  auto status = CompileImpl(ptr->lhs);
  if (!status.ok()) return status;
  status.Update(CompileImpl(ptr->index));
  if (!status.ok()) return status;

  Emit(Opcode::kIndex);
  return kOkStatus;
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

  return kOkStatus;
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

  return kOkStatus;
}

absl::Status Compiler::CompileIdentifier(const ExprNode& expr) {
  const auto name = expr.TokenLiteral();
  const auto symbol = CurrTable().Resolve(name);
  if (!symbol.has_value()) {
    return MakeError("Undefined variable " + name);
  }
  LoadSymbol(*symbol);
  return kOkStatus;
}

absl::Status Compiler::CompileFuncLiteral(const ExprNode& expr) {
  EnterScope();

  const auto* ptr = expr.PtrCast<FuncLiteral>();
  CHECK_NOTNULL(ptr);

  // After entering a new scope and right before compiling the functions' body
  // we define each parameter in the scope of the function.
  // This allows the symbol table to resolve the new refernces and treat them as
  // locals when compiling the function's body
  for (const auto& param : ptr->params) {
    CurrTable().Define(param.String());
  }

  // Compile function body
  auto status = CompileImpl(ptr->body);
  if (!status.ok()) return status;

  if (ScopedLast().op == Opcode::kPop) {
    // Replace last pop with return
    const auto last_pos = ScopedLast().pos;
    ReplaceInstruction(last_pos, Encode(Opcode::kReturnVal));
    ScopedLast().op = Opcode::kReturnVal;
  }

  if (ScopedLast().op != Opcode::kReturnVal) Emit(Opcode::kReturn);

  const auto num_locals = CurrTable().NumDefs();
  const auto num_params = ptr->params.size();
  auto ins = ExitScope();

  Emit(Opcode::kConst,
       static_cast<int>(
           AddConstant(CompiledObj({std::move(ins), num_locals, num_params}))));

  return status;
}

absl::Status Compiler::CompileLetStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<LetStmt>();
  CHECK_NOTNULL(ptr);

  auto status = CompileImpl(ptr->expr);
  if (!status.ok()) return status;

  // Add to symbol table
  const auto& symbol = CurrTable().Define(ptr->name.value);
  const auto index = static_cast<int>(symbol.index);
  if (symbol.IsGlobal()) {
    Emit(Opcode::kSetGlobal, index);
  } else {
    Emit(Opcode::kSetLocal, index);
  }
  return kOkStatus;
}

absl::Status Compiler::CompileExprStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<ExprStmt>();
  CHECK_NOTNULL(ptr);

  auto status = CompileImpl(ptr->expr);
  if (!status.ok()) return status;

  Emit(Opcode::kPop);
  return kOkStatus;
}

absl::Status Compiler::CompileBlockStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<BlockStmt>();
  CHECK_NOTNULL(ptr);

  absl::Status status = kOkStatus;

  for (const auto& st : ptr->statements) {
    status.Update(CompileImpl(st));
    if (!status.ok()) return status;
  }

  return status;
}

absl::Status Compiler::CompileReturnStmt(const StmtNode& stmt) {
  const auto* ptr = stmt.PtrCast<ReturnStmt>();
  CHECK_NOTNULL(ptr);
  auto status = CompileImpl(ptr->expr);
  if (!status.ok()) return status;

  Emit(Opcode::kReturnVal);
  return kOkStatus;
}

void Compiler::LoadSymbol(const Symbol& symbol) {
  const auto index = static_cast<int>(symbol.index);
  switch (symbol.scope) {
    case SymbolScope::kGlobal:
      Emit(Opcode::kGetGlobal, index);
      break;
    case SymbolScope::kLocal:
      Emit(Opcode::kGetLocal, index);
      break;
    case SymbolScope::kBuiltin:
      Emit(Opcode::kGetBuiltin, index);
      break;
    default:
      // Shouldn't reach here
      CHECK(false) << "should not reach here";
      return;
  }
}

}  // namespace monkey
