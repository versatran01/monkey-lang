#include "monkey/compiler.h"

#include <glog/logging.h>

namespace monkey {

absl::StatusOr<Bytecode> Compiler::Compile(const Program& program) {
  auto _ = timers_.Scoped("CompileProgram");

  for (const auto& stmt : program.statements) {
    auto status = CompileImpl(stmt);
    if (!status.ok()) return status;
  }

  return Bytecode{std::move(ins_), std::move(consts_)};
}

absl::Status Compiler::CompileImpl(const AstNode& node) {
  CHECK_NE(node.Type(), NodeType::kProgram);

  switch (node.Type()) {
    case NodeType::kExprStmt: {
      const auto* ptr = node.PtrCast<ExprStmt>();
      auto status = CompileImpl(ptr->expr);
      if (!status.ok()) return status;

      Emit(Opcode::kPop);
      break;
    }
    case NodeType::kPrefixExpr: {
      const auto* ptr = node.PtrCast<PrefixExpr>();
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

      break;
    }
    case NodeType::kInfixExpr: {
      const auto* ptr = node.PtrCast<InfixExpr>();
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
      break;
    }
    case NodeType::kIntLiteral: {
      const auto obj = ToIntObj(node);
      Emit(Opcode::kConst, {AddConstant(obj)});
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

int Compiler::AddConstant(const Object& obj) {
  consts_.push_back(obj);
  return static_cast<int>(consts_.size()) - 1;
}

int Compiler::AddInstruction(const Instruction& ins) {
  const auto pos = static_cast<int>(ins_.NumBytes());
  ins_.Append(ins);
  return pos;
}

int Compiler::Emit(Opcode op, const std::vector<int>& operands) {
  const auto ins = Encode(op, operands);
  return AddInstruction(ins);
}

}  // namespace monkey
