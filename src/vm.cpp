#include "monkey/vm.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ins.bytes[ip]);

    auto status = absl::OkStatus();

    switch (op) {
      case Opcode::kConst: {
        const auto const_index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;

        Push(bc.consts[const_index]);
        break;
      }
      case Opcode::kAdd:
      case Opcode::kSub:
      case Opcode::kMul:
      case Opcode::kDiv: {
        status = ExecBinaryOp(op);
        break;
      }
      case Opcode::kTrue:
        Push(BoolObj(true));
        break;
      case Opcode::kFalse:
        Push(BoolObj(false));
        break;
      case Opcode::kEq:
      case Opcode::kNe:
      case Opcode::kGt: {
        status = ExecComparison(op);
        break;
      }
      case Opcode::kBang: {
        status = ExecBangOp();
        break;
      }
      case Opcode::kMinus: {
        status = ExecMinusOp();
        break;
      }
      case Opcode::kPop: {
        Pop();
        break;
      }
      default:
        return MakeError("Unhandled Opcode: " + Repr(op));
    }

    // Check status and early return
    if (!status.ok()) return status;
  }

  return absl::OkStatus();
}

absl::Status VirtualMachine::ExecBinaryOp(Opcode op) {
  const auto rhs = Pop();
  const auto lhs = Pop();

  if (ObjOfSameType(ObjectType::kInt, lhs, rhs)) {
    return ExecIntBinaryOp(lhs, op, rhs);
  }

  return MakeError(fmt::format("Unsupported types for binary operations: {} {}",
                               lhs.Type(),
                               rhs.Type()));
}

absl::Status VirtualMachine::ExecIntBinaryOp(const Object& lhs,
                                             Opcode op,
                                             const Object& rhs) {
  const auto lv = lhs.Cast<IntType>();
  const auto rv = rhs.Cast<IntType>();

  IntType res{};
  switch (op) {
    case Opcode::kAdd:
      res = lv + rv;
      break;
    case Opcode::kSub:
      res = lv - rv;
      break;
    case Opcode::kMul:
      res = lv * rv;
      break;
    case Opcode::kDiv:
      res = lv / rv;
      break;
    default:
      return MakeError("Unknown integer operator: " + Repr(op));
  }

  Push(IntObj(res));
  return absl::OkStatus();
}

absl::Status VirtualMachine::ExecComparison(Opcode op) {
  const auto rhs = Pop();
  const auto lhs = Pop();

  if (ObjOfSameType(ObjectType::kInt, lhs, rhs)) {
    return ExecIntComparison(lhs, op, rhs);
  }

  // Otherwise must be bool
  CHECK(ObjOfSameType(ObjectType::kBool, lhs, rhs));
  switch (op) {
    case Opcode::kEq:
      Push(BoolObj(lhs.Cast<BoolType>() == rhs.Cast<BoolType>()));
      break;
    case Opcode::kNe:
      Push(BoolObj(lhs.Cast<BoolType>() != rhs.Cast<BoolType>()));
      break;
    default:
      return MakeError(fmt::format(
          "Unknown operator: {} ({} {})", op, lhs.Type(), rhs.Type()));
  }

  return absl::OkStatus();
}

absl::Status VirtualMachine::ExecIntComparison(const Object& lhs,
                                               Opcode op,
                                               const Object& rhs) {
  const auto lv = lhs.Cast<IntType>();
  const auto rv = rhs.Cast<IntType>();

  BoolType res{};
  switch (op) {
    case Opcode::kEq:
      res = lv == rv;
      break;
    case Opcode::kNe:
      res = lv != rv;
      break;
    case Opcode::kGt:
      res = lv > rv;
      break;
    default:
      return MakeError("Unknown opeartor: " + Repr(op));
  }

  Push(BoolObj(res));
  return absl::OkStatus();
}

absl::Status VirtualMachine::ExecBangOp() {
  const auto obj = Pop();

  if (obj.Type() == ObjectType::kBool && !obj.Cast<BoolType>()) {
    Push(BoolObj(true));
  } else {
    Push(BoolObj(false));
  }

  return absl::OkStatus();
}

absl::Status VirtualMachine::ExecMinusOp() {
  const auto obj = Pop();
  if (obj.Type() != ObjectType::kInt) {
    return MakeError("Unsupported type for negation: " + Repr(obj.Type()));
  }

  Push(IntObj(-obj.Cast<IntType>()));
  return absl::OkStatus();
}

const Object& VirtualMachine::Top() const {
  CHECK_GT(sp, 0) << "Calling Top() when Stack is empty";
  return stack[sp - 1];
}

const Object& VirtualMachine::Last() const { return stack.at(sp); }

Object VirtualMachine::Pop() {
  Object o = Top();  // Will check empty stack in Top();
  --sp;
  return o;
}

void VirtualMachine::Push(Object obj) {
  if (sp == stack.size()) {
    stack.push_back(std::move(obj));
  } else {
    stack[sp] = std::move(obj);
  }

  ++sp;
  CHECK_LE(sp, stack.size());
}

}  // namespace monkey
