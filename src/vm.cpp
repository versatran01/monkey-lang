#include "monkey/vm.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  frames_.push({CompiledFunc{bc.ins}});

  auto status = kOkStatus;

  while (CurrFrame().ip < CurrFrame().Ins().NumBytes()) {
    const auto& ins = CurrFrame().Ins();
    auto& ip = CurrFrame().ip;

    const auto op = ToOpcode(ins.ByteAt(ip));

    switch (op) {
      case Opcode::kConst: {
        const auto const_index = ReadUint16(ins.BytePtr(ip + 1));
        ip += 2;

        PushStack(bc.consts[const_index]);
        break;
      }
      case Opcode::kNull:
        PushStack(NullObj());
        break;
      case Opcode::kAdd:
      case Opcode::kSub:
      case Opcode::kMul:
      case Opcode::kDiv: {
        status = ExecBinaryOp(op);
        break;
      }
      case Opcode::kTrue:
        PushStack(BoolObj(true));
        break;
      case Opcode::kFalse:
        PushStack(BoolObj(false));
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
        PopStack();
        break;
      }
      case Opcode::kIndex: {
        auto index = PopStack();
        auto lhs = PopStack();
        status = ExecIndexExpr(lhs, index);
        break;
      }
      case Opcode::kJump: {
        size_t pos = ReadUint16(ins.BytePtr(ip + 1));
        ip = pos - 1;  // the loop will increment ip, so -1
        break;
      }
      case Opcode::kJumpNotTrue: {
        size_t pos = ReadUint16(ins.BytePtr(ip + 1));
        ip += 2;
        const auto cond = PopStack();
        if (!IsObjTruthy(cond)) ip = pos - 1;
        break;
      }
      case Opcode::kSetGlobal: {
        auto index = ReadUint16(ins.BytePtr(ip + 1));
        ip += 2;
        globals_[index] = PopStack();
        break;
      }
      case Opcode::kGetGlobal: {
        auto index = ReadUint16(ins.BytePtr(ip + 1));
        ip += 2;
        PushStack(globals_.at(index));
        break;
      }
      case Opcode::kArray: {
        const auto size = ReadUint16(ins.BytePtr(ip + 1));
        ip += 2;
        PushStack(BuildArray(size));
        break;
      }
      case Opcode::kDict: {
        const auto size = ReadUint16(ins.BytePtr(ip + 1));
        ip += 2;
        auto obj = BuildDict(size);
        if (IsObjError(obj)) return MakeError(obj.Inspect());
        PushStack(std::move(obj));
        break;
      }
      case Opcode::kCall: {
        // This will be popped when function returns
        auto obj = Top();

        if (obj.Type() != ObjectType::kCompiled) {
          status.Update(MakeError("calling non-function: " + Repr(obj.Type())));
        } else {
          PushFrame(Frame{std::move(obj.MutCast<CompiledFunc>())});
        }
        break;
      }
      case Opcode::kReturnVal: {
        auto ret = PopStack();
        PopFrame();
        ReplaceStackTop(std::move(ret));
        continue;  // we do not want to increment ip if we just popped the frame
      }
      case Opcode::kReturn: {
        PopFrame();
        ReplaceStackTop(NullObj());
        continue;
      }
      default:
        return MakeError("Unhandled Opcode: " + Repr(op));
    }

    // Check status and early return
    if (!status.ok()) return status;

    ++ip;
  }

  return status;
}

absl::Status VirtualMachine::ExecBinaryOp(Opcode op) {
  const auto rhs = PopStack();
  const auto lhs = PopStack();

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

  PushStack(IntObj(res));
  return kOkStatus;
}

absl::Status VirtualMachine::ExecComparison(Opcode op) {
  const auto rhs = PopStack();
  const auto lhs = PopStack();

  if (ObjOfSameType(ObjectType::kInt, lhs, rhs)) {
    return ExecIntComp(lhs, op, rhs);
  }

  // Otherwise must be bool
  CHECK(ObjOfSameType(ObjectType::kBool, lhs, rhs));
  switch (op) {
    case Opcode::kEq:
      PushStack(BoolObj(lhs.Cast<BoolType>() == rhs.Cast<BoolType>()));
      break;
    case Opcode::kNe:
      PushStack(BoolObj(lhs.Cast<BoolType>() != rhs.Cast<BoolType>()));
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

  PushStack(BoolObj(res));
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
    PushStack(NullObj());
  } else {
    PushStack(it->second);
  }
  return kOkStatus;
}

absl::Status VirtualMachine::ExecArrayIndex(const Object& lhs,
                                            const Object& index) {
  const auto& array = lhs.Cast<Array>();
  const auto i = index.Cast<IntType>();
  const auto size = array.size();

  if (i < 0 || i >= size) {
    PushStack(NullObj());
  } else {
    PushStack(array[i]);
  }
  return kOkStatus;
}

Object VirtualMachine::BuildArray(size_t size) {
  Array arr;
  arr.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    arr.push_back(PopStack());
  }
  std::reverse(arr.begin(), arr.end());
  return ArrayObj(std::move(arr));
}

Object VirtualMachine::BuildDict(size_t size) {
  Dict dict;

  for (size_t i = 0; i < size; i += 2) {
    auto value = PopStack();
    auto key = PopStack();

    if (!IsObjHashable(key)) {
      return ErrorObj("unusable as hash key: " + Repr(key.Type()));
    }

    dict[key] = value;
  }

  return DictObj(std::move(dict));
}

absl::Status VirtualMachine::ExecStrBinaryOp(const Object& lhs,
                                             Opcode op,
                                             const Object& rhs) {
  const auto& lv = lhs.Cast<StrType>();
  const auto& rv = rhs.Cast<StrType>();

  if (op != Opcode::kAdd) {
    return MakeError("unknown string operator: " + Repr(op));
  }

  PushStack(StrObj(lv + rv));
  return kOkStatus;
}

absl::Status VirtualMachine::ExecBangOp() {
  const auto obj = PopStack();

  if (obj.Type() == ObjectType::kBool) {
    PushStack(BoolObj(!obj.Cast<BoolType>()));
  } else if (obj.Type() == ObjectType::kNull) {
    PushStack(BoolObj(true));
  } else {
    PushStack(BoolObj(false));
  }

  return kOkStatus;
}

absl::Status VirtualMachine::ExecMinusOp() {
  const auto obj = PopStack();
  if (obj.Type() != ObjectType::kInt) {
    return MakeError("Unsupported type for negation: " + Repr(obj.Type()));
  }

  PushStack(IntObj(-obj.Cast<IntType>()));
  return kOkStatus;
}

const Object& VirtualMachine::Top() const {
  CHECK(!stack_.empty());
  return stack_.top();
}

const Object& VirtualMachine::PopStack() {
  CHECK(!stack_.empty());
  last_ = std::move(stack_.top());
  stack_.pop();
  return last_;
}

void VirtualMachine::PushStack(Object obj) { stack_.emplace(std::move(obj)); }

void VirtualMachine::ReplaceStackTop(Object obj) {
  last_ = Top();
  stack_.top() = std::move(obj);
}

Frame VirtualMachine::PopFrame() {
  CHECK(!frames_.empty());
  auto frame = std::move(frames_.top());
  frames_.pop();
  return frame;
}

void VirtualMachine::PushFrame(Frame frame) {
  frames_.emplace(std::move(frame));
}

}  // namespace monkey
