#include "monkey/code.h"

#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>
#include <glog/logging.h>

namespace monkey {

namespace {

const absl::flat_hash_map<Opcode, Definition> gOpcodeDefinitions = {
    {Opcode::Const, {"OpConst", {2}}},

};

void PutUint16(uint8_t* dst, uint16_t n) {
  n = (n >> 8) | (n << 8);
  std::memcpy(dst, &n, sizeof(uint16_t));
}

uint16_t ReadUint16(const uint8_t* src) {
  uint16_t n = *reinterpret_cast<const uint16_t*>(src);
  n = (n >> 8) | (n << 8);
  return n;
}

std::string FormatInstruction(const Definition& def,
                              const std::vector<int>& operands) {
  const auto num_operands = operands.size();
  CHECK_EQ(num_operands, def.NumOperands());

  switch (num_operands) {
    case 1:
      return fmt::format("{} {}", def.name, operands[0]);
    default:
      CHECK(false) << "Should not reach here";
  }

  return fmt::format("ERROR: unhandled operandCount for {}\n", def.name);
}

}  // namespace

Definition LookupDefinition(Opcode op) { return gOpcodeDefinitions.at(op); }

std::string Instruction::String() const {
  std::string str;

  size_t i = 0;
  while (i < bytes.size()) {
    const auto def = LookupDefinition(ToOpcode(bytes[i]));
    const auto dec = Decode(def, *this, i + 1);  // +1 is to skip the opcode
    str += fmt::format("{:04d} {}\n", i, FormatInstruction(def, dec.operands));
    i += 1 + dec.nbytes;  // opcode (1) + num bytes
  }
  return str;
}

std::ostream& operator<<(std::ostream& os, const Instruction& ins) {
  return os << ins.String();
}

Instruction ConcatInstructions(const std::vector<Instruction>& instrs) {
  Instruction out;
  for (const auto& ins : instrs) {
    out.bytes.insert(out.bytes.end(), ins.bytes.begin(), ins.bytes.end());
  }
  return out;
}

Instruction Encode(Opcode op, const std::vector<int>& operands) {
  //  const auto it = gOpcodeDefinitions.find(op);
  //  if (it == gOpcodeDefinitions.end()) {
  //    return {};
  //  }
  //  const auto& def = it->second;
  const auto def = LookupDefinition(op);

  CHECK_EQ(def.NumOperands(), operands.size());
  const auto total_bytes = def.SumOperandBytes() + 1;

  Instruction ins;
  ins.bytes.resize(total_bytes);
  ins.bytes[0] = ToByte(op);

  size_t offset = 1;

  for (size_t i = 0; i < operands.size(); ++i) {
    const auto nbytes = def.operand_bytes[i];
    switch (nbytes) {
      case 2:
        PutUint16(&ins.bytes[offset], static_cast<uint16_t>(operands[i]));
        break;
      default:
        CHECK(false) << "Should not reach here";
    }
    offset += nbytes;
  }

  return ins;
}

Decoded Decode(const Definition& def, const Instruction& ins, int offset) {
  Decoded dec;
  dec.operands.reserve(def.NumOperands());

  for (const auto& width : def.operand_bytes) {
    const auto start = dec.nbytes + offset;
    CHECK_LE(start + width, ins.size());

    switch (width) {
      case 2:
        dec.operands.push_back(ReadUint16(&ins.bytes[dec.nbytes + offset]));
        break;
      default:
        CHECK(false) << "Should not reach here";
    }
    dec.nbytes += width;
  }

  return dec;
}

size_t Definition::SumOperandBytes() const {
  return std::accumulate(
      operand_bytes.cbegin(), operand_bytes.cend(), size_t{0});
}

}  // namespace monkey
