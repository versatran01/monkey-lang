#include "monkey/compiler.h"

#include <glog/logging.h>

namespace monkey {

absl::StatusOr<Bytecode> Compiler::Compile(const AstNode& node) {
  const auto status = CompileImpl(node);
  if (!status.ok()) {
    return status;
  }

  return Bytecode{std::move(ins_), std::move(consts_)};
}

absl::Status Compiler::CompileImpl(const AstNode& node) {
  switch (node.Type()) {
    case (NodeType::kProgram): {
      const auto* ptr = node.PtrCast<Program>();
      for (const auto& stmt : ptr->statements) {
        const auto status = CompileImpl(stmt);
        if (!status.ok()) {
          return status;
        }
      }
      return absl::OkStatus();
    }
    case (NodeType::kExprStmt): {
      const auto* ptr = node.PtrCast<ExprStmt>();
      return CompileImpl(ptr->expr);
    }
    case (NodeType::kInfixExpr): {
      const auto* ptr = node.PtrCast<InfixExpr>();
      auto status = CompileImpl(ptr->lhs);
      if (!status.ok()) {
        return status;
      }

      status = CompileImpl(ptr->rhs);
      if (!status.ok()) {
        return status;
      }
      return absl::OkStatus();
    }
    case (NodeType::kIntLiteral): {
      const auto obj = IntObj(node);
      Emit(Opcode::kConst, {AddConstant(obj)});
      return absl::OkStatus();
    }
    default:
      CHECK(false) << "Should not reach here";
  }
  return absl::InternalError("Should not reach here");
}

int Compiler::AddConstant(const Object& obj) {
  consts_.push_back(obj);
  return static_cast<int>(consts_.size()) - 1;
}

int Compiler::AddInstruction(const Instruction& ins) {
  const auto pos = static_cast<int>(ins_.size());
  ins_.Append(ins);
  return pos;
}

int Compiler::Emit(Opcode op, const std::vector<int>& operands) {
  const auto ins = Encode(op, operands);
  return AddInstruction(ins);
}

}  // namespace monkey
