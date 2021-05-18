#include "monkey/vm.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ins.bytes[ip]);

    switch (op) {
      case (Opcode::kConst): {
        const auto const_index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;

        Push(bc.consts[const_index]);
        break;
      }
      case (Opcode::kAdd):
      case (Opcode::kSub):
      case (Opcode::kMul):
      case (Opcode::kDiv): {
        const auto status = ExecBinaryOp(op);
        if (!status.ok()) {
          return status;
        }
        break;
      }
      case (Opcode::kTrue): {
        Push(BoolObj(true));
        break;
      }
      case (Opcode::kFalse): {
        Push(BoolObj(false));
        break;
      }
      case (Opcode::kPop): {
        Pop();
        break;
      }
      default:
        return MakeError("Unhandled Opcode: " + ToString(op));
    }
  }

  return absl::OkStatus();
}

absl::Status VirtualMachine::ExecBinaryOp(Opcode op) {
  const auto rhs = Pop();
  const auto lhs = Pop();

  if (lhs.Type() == ObjectType::kInt && rhs.Type() == ObjectType::kInt) {
    return ExecIntBinaryOp(op, lhs, rhs);
  }

  return MakeError(fmt::format("Unsupported types for binary operations: {} {}",
                               lhs.Type(),
                               rhs.Type()));
}

absl::Status VirtualMachine::ExecIntBinaryOp(Opcode op,
                                             const Object& lhs,
                                             const Object& rhs) {
  const auto lv = lhs.Cast<IntType>();
  const auto rv = rhs.Cast<IntType>();

  IntType res{};
  switch (op) {
    case (Opcode::kAdd):
      res = lv + rv;
      break;
    case (Opcode::kSub):
      res = lv - rv;
      break;
    case (Opcode::kMul):
      res = lv * rv;
      break;
    case (Opcode::kDiv):
      res = lv / rv;
      break;
    default:
      return MakeError("Unknown integer operator: " + ToString(op));
  }

  Push(IntObj(res));
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
