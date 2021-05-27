#include "monkey/vm.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  auto status = kOkStatus;

  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ByteAt(ip));

    switch (op) {
      case Opcode::kConst: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        const auto const_index = ReadUint16(bc.BytePtr(ip + 1));
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
      case Opcode::kIndex: {
        auto index = Pop();
        auto lhs = Pop();
        status = ExecIndexExpr(lhs, index);
        break;
      }
      case Opcode::kJump: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        size_t pos = ReadUint16(bc.BytePtr(ip + 1));
        ip = pos - 1;  // the loop will increment ip, so -1
        break;
      }
      case Opcode::kJumpNotTrue: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        size_t pos = ReadUint16(bc.BytePtr(ip + 1));
        ip += 2;
        const auto cond = Pop();
        if (!IsObjTruthy(cond)) ip = pos - 1;
        break;
      }
      case Opcode::kSetGlobal: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        auto index = ReadUint16(bc.BytePtr(ip + 1));
        ip += 2;
        globals_[index] = Pop();
        break;
      }
      case Opcode::kGetGlobal: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        auto index = ReadUint16(bc.BytePtr(ip + 1));
        ip += 2;
        Push(globals_.at(index));
        break;
      }
      case Opcode::kArray: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        const auto size = ReadUint16(bc.BytePtr(ip + 1));
        ip += 2;
        const auto obj = BuildArray(sp_ - size, sp_);
        sp_ -= size;
        Push(obj);
        break;
      }
      case Opcode::kDict: {
        CHECK_LT(ip + 1, bc.ins.NumBytes());
        const auto size = ReadUint16(bc.BytePtr(ip + 1));
        ip += 2;
        const auto obj = BuildDict(sp_ - size, sp_);
        if (IsObjError(obj)) return MakeError(obj.Inspect());
        sp_ -= size;
        Push(obj);
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
  }

  if (ObjOfSameType(ObjectType::kStr, lhs, rhs)) {
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

absl::Status VirtualMachine::ExecIndexExpr(const Object& lhs,
                                           const Object& index) {
  if (lhs.Type() == ObjectType::kArray && index.Type() == ObjectType::kInt) {
    return ExecArrayIndex(lhs, index);
  }

  if (lhs.Type() == ObjectType::kDict) {
    return ExecDictIndex(lhs, index);
  }

  return MakeError("index operator not supported: " + Repr(lhs.Type()));
}

absl::Status VirtualMachine::ExecDictIndex(const Object& lhs,
                                           const Object& index) {
  const auto& dict = lhs.Cast<Dict>();
  if (!IsObjHashable(index)) {
    return MakeError("unusable as hash key: " + Repr(index.Type()));
  }

  const auto it = dict.find(index);
  if (it == dict.end()) {
    Push(NullObj());
  } else {
    Push(it->second);
  }
  return kOkStatus;
}

absl::Status VirtualMachine::ExecArrayIndex(const Object& lhs,
                                            const Object& index) {
  const auto& array = lhs.Cast<Array>();
  const auto i = index.Cast<IntType>();
  const auto size = array.size();

  if (i < 0 || i >= size) {
    Push(NullObj());
  } else {
    Push(array[i]);
  }
  return kOkStatus;
}

Object VirtualMachine::BuildArray(size_t start, size_t end) const {
  return ArrayObj({stack_.begin() + start, stack_.begin() + end});
}

Object VirtualMachine::BuildDict(size_t start, size_t end) const {
  Dict dict;

  for (size_t i = start; i < end; i += 2) {
    const auto& key = stack_.at(i);
    if (!IsObjHashable(key)) {
      return ErrorObj("unusable as hash key: " + Repr(key.Type()));
    }
    dict[key] = stack_.at(i + 1);
  }

  return DictObj(std::move(dict));
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

void VirtualMachine::Push(const Object& obj) {
  if (sp_ == stack_.size()) {
    stack_.push_back(obj);
  } else {
    stack_[sp_] = obj;
  }

  ++sp_;
  CHECK_LE(sp_, stack_.size());
}

}  // namespace monkey
