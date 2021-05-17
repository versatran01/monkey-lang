#include "monkey/vm.h"

namespace monkey {

void VirtualMachine::Run(const Bytecode& bc) {
  for (size_t ip = 0; ip < bc.ins.NumBytes(); ++ip) {
    const auto op = ToOpcode(bc.ins.bytes[ip]);

    switch (op) {
      case (Opcode::kConst): {
        auto const_index = ReadUint16(&bc.ins.bytes[ip + 1]);
        ip += 2;

        stack.push(bc.consts[const_index]);
      }
    }
  }
}

}  // namespace monkey
