#include "monkey/vm.h"

#include <glog/logging.h>

namespace monkey {

absl::Status VirtualMachine::Run(const Bytecode& bc) {
  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ins.bytes[ip]);

    switch (op) {
      case (Opcode::kConst): {
        auto const_index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;

        stack.push(bc.consts[const_index]);
        break;
      }
      case (Opcode::kAdd): {
        const auto rhs = Pop();
        const auto lhs = Pop();
        stack.push(IntObj(lhs.Cast<IntType>() + rhs.Cast<IntType>()));
        break;
      }
      case (Opcode::kPop): {
        Pop();
        break;
      }
      default:
        return absl::InternalError("Unhandled Opcode: " + ToString(op));
    }
  }

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

}  // namespace monkey
