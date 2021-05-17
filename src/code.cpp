#include "monkey/code.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/core.h>
#include <glog/logging.h>

namespace monkey {

namespace {

const absl::flat_hash_map<Opcode, Definition> gOpcodeDefinitions = {
    {Opcode::kConst, {"OpConst", {2}}},
    {Opcode::kAdd, {"OpAdd", {}}},

};

std::string FormatInstruction(const Definition& def,
                              const std::vector<int>& operands) {
  const auto num_operands = operands.size();
  CHECK_EQ(num_operands, def.NumOperands());

  switch (num_operands) {
    case 0:
      return def.name;
    case 1:
      return fmt::format("{} {}", def.name, operands[0]);
    default:
      CHECK(false) << "Should not reach here";
  }

  return fmt::format("ERROR: unhandled operandCount for {}\n", def.name);
}

}  // namespace

std::ostream& operator<<(std::ostream& os, Opcode op) {
  return os << gOpcodeDefinitions.at(op).name;
}

Definition LookupDefinition(Opcode op) { return gOpcodeDefinitions.at(op); }

void Instruction::Append(const Instruction& ins) {
  bytes.insert(bytes.end(), ins.bytes.cbegin(), ins.bytes.cend());
  ++num_ops;
}

std::string Instruction::String() const {
  std::vector<std::string> strs;
  strs.reserve(NumOps());

  size_t i = 0;
  while (i < bytes.size()) {
    const auto def = LookupDefinition(ToOpcode(bytes[i]));
    const auto dec = Decode(def, *this, i + 1);  // +1 is to skip the opcode
    strs.push_back(
        fmt::format("{:04d} {}", i, FormatInstruction(def, dec.operands)));
    i += size_t{1} + dec.nbytes;  // opcode (1) + num bytes
  }
  return absl::StrJoin(strs, "\n");
}

std::ostream& operator<<(std::ostream& os, const Instruction& ins) {
  return os << ins.String();
}

Instruction ConcatInstructions(const std::vector<Instruction>& instrs) {
  Instruction out;
  for (const auto& ins : instrs) {
    out.Append(ins);
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
  ins.num_ops = 1;
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

Decoded Decode(const Definition& def, const Instruction& ins, size_t offset) {
  Decoded dec;
  dec.operands.reserve(def.NumOperands());

  for (const auto& nbytes : def.operand_bytes) {
    const auto start = dec.nbytes + offset;
    CHECK_LE(start + nbytes, ins.NumBytes());

    switch (nbytes) {
      case 2:
        dec.operands.push_back(ReadUint16(&ins.bytes[dec.nbytes + offset]));
        break;
      default:
        CHECK(false) << "Should not reach here";
    }
    dec.nbytes += nbytes;
  }

  return dec;
}

size_t Definition::SumOperandBytes() const {
  return std::accumulate(
      operand_bytes.cbegin(), operand_bytes.cend(), size_t{0});
}

}  // namespace monkey
