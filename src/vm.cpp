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
      case Opcode::kSetLocal: {
        const size_t index = ins.ByteAt(ip + 1);
        ip += 1;
        const auto& frame = CurrFrame();
        stack_.at(frame.bp + index) = PopStack();
        break;
      }
      case Opcode::kGetLocal: {
        const size_t index = ins.ByteAt(ip + 1);
        ip += 1;
        const auto& frame = CurrFrame();
        PushStack(stack_.at(frame.bp + index));
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
        const size_t num_args = ins.ByteAt(ip + 1);
        ip += 1;
        const Object& obj = StackTop(num_args);
        status.Update(ExecFuncCall(obj, num_args));
        break;
      }
      case Opcode::kReturnVal: {
        auto ret = PopStack();
        const auto frame = PopFrame();
        sp_ = frame.bp - 1;  // restore sp
        PushStack(std::move(ret));
        continue;  // we do not want to increment ip if we just popped the frame
      }
      case Opcode::kReturn: {
        const auto frame = PopFrame();
        sp_ = frame.bp - 1;
        PushStack(NullObj());
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

absl::Status VirtualMachine::ExecFuncCall(const Object& obj, size_t num_args) {
  auto status = kOkStatus;
  // This obj will be popped of the stack when function returns
  // Instead of grabbing the function off the top of the stack, we
  // calculate its position by decoding the operand and subtracting it
  // from the top

  if (obj.Type() != ObjectType::kCompiled) {
    status.Update(MakeError("calling non-function: " + Repr(obj.Type())));
    return status;
  }

  const auto& func = obj.Cast<CompiledFunc>();

  if (num_args != func.num_params) {
    status.Update(
        MakeError(fmt::format("wrong number of arguments: want={}, got={}",
                              func.num_params,
                              num_args)));
    return status;
  }

  PushFrame(Frame{func, sp_ - num_args});
  AllocateLocal(func.num_locals);

  return status;
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

const Object& VirtualMachine::StackTop(size_t offset) const {
  CHECK_GT(sp_, 0) << "Calling Top() when Stack is empty";
  return stack_[sp_ - 1 - offset];
}

const Object& VirtualMachine::Last() const {
  CHECK_LT(sp_, stack_.size());
  return stack_.at(sp_);
}

Object VirtualMachine::PopStack() {
  Object o = StackTop();  // Will check empty in Top();
  --sp_;
  return o;
}

void VirtualMachine::PushStack(Object obj) {
  if (sp_ == stack_.size()) {
    stack_.emplace_back(std::move(obj));
  } else {
    stack_.at(sp_) = std::move(obj);
  }

  ++sp_;
  CHECK_LE(sp_, stack_.size());
}

void VirtualMachine::ReplaceStackTop(Object obj) {
  CHECK_GT(sp_, 0) << "Calling Top() when Stack is empty";
  stack_.at(sp_ - 1) = std::move(obj);
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

void VirtualMachine::AllocateLocal(size_t num_locals) {
  if (sp_ + num_locals >= stack_.size()) {
    stack_.resize(sp_ + num_locals);
  }
  sp_ += num_locals;
}

}  // namespace monkey
