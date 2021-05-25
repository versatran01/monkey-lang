#include "monkey/vm.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  auto status = kOkStatus;

  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ins.bytes[ip]);

    switch (op) {
      case Opcode::kConst: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        const auto const_index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;

        Push(bc.consts[const_index]);
        break;
      }
      case Opcode::kNull:
        Push(NullObj());
        break;
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
      case Opcode::kJump: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        size_t pos = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip = pos - 1;  // the loop will increment ip, so -1
        break;
      }
      case Opcode::kJumpNotTrue: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        size_t pos = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;
        const auto cond = Pop();
        if (!IsObjTruthy(cond)) ip = pos - 1;
        break;
      }
      case Opcode::kSetGlobal: {
        auto index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;
        globals_[index] = Pop();
        break;
      }
      case Opcode::kGetGlobal: {
        auto index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;
        Push(globals_.at(index));
        break;
      }
      default:
        return MakeError("Unhandled Opcode: " + Repr(op));
    }

    // Check status and early return
    if (!status.ok()) return status;
  }

  return status;
}

absl::Status VirtualMachine::ExecBinaryOp(Opcode op) {
  const auto rhs = Pop();
  const auto lhs = Pop();

  if (ObjOfSameType(ObjectType::kInt, lhs, rhs)) {
    return ExecIntBinaryOp(lhs, op, rhs);
  } else if (ObjOfSameType(ObjectType::kStr, lhs, rhs)) {
    return ExecStrBinaryOp(lhs, op, rhs);
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
  return kOkStatus;
}

absl::Status VirtualMachine::ExecComparison(Opcode op) {
  const auto rhs = Pop();
  const auto lhs = Pop();

  if (ObjOfSameType(ObjectType::kInt, lhs, rhs)) {
    return ExecIntComp(lhs, op, rhs);
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

  return kOkStatus;
}

absl::Status VirtualMachine::ExecIntComp(const Object& lhs,
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
      return MakeError("Unknown operator: " + Repr(op));
  }

  Push(BoolObj(res));
  return kOkStatus;
}

absl::Status VirtualMachine::ExecStrBinaryOp(const Object& lhs,
                                             Opcode op,
                                             const Object& rhs) {
  const auto lv = lhs.Cast<StrType>();
  const auto rv = rhs.Cast<StrType>();

  if (op != Opcode::kAdd) {
    return MakeError("unknown string operator: " + Repr(op));
  }

  Push(StrObj(lv + rv));
  return kOkStatus;
}

absl::Status VirtualMachine::ExecBangOp() {
  const auto obj = Pop();

  if (obj.Type() == ObjectType::kBool) {
    Push(BoolObj(!obj.Cast<BoolType>()));
  } else if (obj.Type() == ObjectType::kNull) {
    Push(BoolObj(true));
  } else {
    Push(BoolObj(false));
  }

  return kOkStatus;
}

absl::Status VirtualMachine::ExecMinusOp() {
  const auto obj = Pop();
  if (obj.Type() != ObjectType::kInt) {
    return MakeError("Unsupported type for negation: " + Repr(obj.Type()));
  }

  Push(IntObj(-obj.Cast<IntType>()));
  return kOkStatus;
}

const Object& VirtualMachine::Top() const {
  CHECK_GT(sp_, 0) << "Calling Top() when Stack is empty";
  return stack_[sp_ - 1];
}

const Object& VirtualMachine::Last() const { return stack_.at(sp_); }

Object VirtualMachine::Pop() {
  Object o = Top();  // Will check empty in Top();
  --sp_;
  return o;
}

void VirtualMachine::Push(Object obj) {
  if (sp_ == stack_.size()) {
    stack_.push_back(std::move(obj));
  } else {
    stack_[sp_] = std::move(obj);
  }

  ++sp_;
  CHECK_LE(sp_, stack_.size());
}

}  // namespace monkey
