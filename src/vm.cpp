#include "monkey/vm.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ins.bytes[ip]);

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
        const auto status = ExecBinaryOp(op);
        if (!status.ok()) {
          return status;
        }
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
        const auto status = ExecComparison(op);
        if (!status.ok()) {
          return status;
        }
        break;
      }
      case Opcode::kPop: {
        Pop();
        break;
      }
      default:
        return MakeError("Unhandled Opcode: " + Repr(op));
    }
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

const Object& VirtualMachine::Top() const {
  CHECK(!stack.empty()) << "Top called when stack is empty";
  return stack.top();
}

Object VirtualMachine::Pop() {
  last_ = Top();
  stack.pop();
  return last_;
}

void VirtualMachine::Push(Object obj) { stack.push(std::move(obj)); }

}  // namespace monkey
