#include "monkey/code.h"

#include <absl/container/flat_hash_map.h>
#include <glog/logging.h>

#include "monkey/endian.h"

namespace monkey {

namespace {

const absl::flat_hash_map<Opcode, Definition> gOpcodeDefinitions = {
    {Opcode::Const, {"OpConst", {2}}},
};

}

Definition LookupDefinition(Opcode op) { return gOpcodeDefinitions.at(op); }

Instruction MakeInstruction(Opcode op, const std::vector<int>& operands) {
  const auto it = gOpcodeDefinitions.find(op);
  if (it == gOpcodeDefinitions.end()) {
    return {};
  }
  const auto& def = it->second;

  CHECK_EQ(def.widths.size(), operands.size());
  auto len = std::accumulate(def.widths.cbegin(), def.widths.cend(), 1);

  Instruction instruction(len);
  instruction[0] = ToByte(op);

  size_t offset = 1;

  for (size_t i = 0; i < operands.size(); ++i) {
    const auto width = def.widths[i];
    switch (width) {
      case 2:
        PutUint16(&instruction[offset], static_cast<uint16_t>(operands[i]));
        break;
      default:
        break;
    }
    offset += width;
  }

  return instruction;
}

}  // namespace monkey
