#include "monkey/instruction.h"

#include <absl/strings/str_join.h>
#include <absl/types/span.h>
#include <fmt/core.h>
#include <glog/logging.h>

namespace monkey {

namespace {

std::string FormatInstruction(const Definition& def,
                              absl::Span<const int> operands) {
  const auto num_operands = operands.size();
  CHECK_EQ(num_operands, def.NumOperands());

  switch (num_operands) {
    case 0:
      return def.name;
    case 1:
      return fmt::format("{} {}", def.name, operands[0]);
    case 2:
      return fmt::format("{} {} {}", def.name, operands[0], operands[1]);
    default:
      CHECK(false) << "Should not reach here";
  }

  return fmt::format("ERROR: unhandled operand count for {}\n", def.name);
}

}  // namespace

void Instruction::Append(const Instruction& ins) {
  // just be sure we call reserve explicitly
  bytes.reserve(bytes.size() + ins.NumBytes());
  bytes.insert(bytes.end(), ins.bytes.cbegin(), ins.bytes.cend());
  ++num_ops;
}

Byte Instruction::PopBack() {
  auto byte = bytes.back();
  bytes.pop_back();
  --num_ops;
  return byte;
}

size_t Instruction::EncodeOpcode(Opcode op, size_t total_bytes) {
  ++num_ops;
  auto nbytes = NumBytes();
  bytes.resize(nbytes + total_bytes);
  bytes[nbytes] = ToByte(op);
  return nbytes + 1;
}

void Instruction::EncodeOperand(size_t offset, size_t nbytes, int operand) {
  CHECK_LE(0, operand);

  switch (nbytes) {
    case 1:
      CHECK_LE(operand, std::numeric_limits<uint8_t>::max());
      bytes[offset] = static_cast<uint8_t>(operand);
      break;
    case 2:
      CHECK_LE(operand, std::numeric_limits<uint16_t>::max());
      PutUint16(&bytes[offset], static_cast<uint16_t>(operand));
      break;
    default:
      CHECK(false) << "Should not reach here";
  }
}

std::string Instruction::Repr() const {
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
  return os << ins.Repr();
}

Instruction ConcatInstructions(const std::vector<Instruction>& instrs) {
  Instruction out;
  for (const auto& ins : instrs) {
    out.Append(ins);
  }
  return out;
}

Instruction Encode(Opcode op, int operand) {
  const auto def = LookupDefinition(op);
  CHECK_EQ(def.NumOperands(), 1) << def;
  const auto total_bytes = def.SumOperandBytes() + 1;

  Instruction ins;
  const auto offset = ins.EncodeOpcode(op, total_bytes);
  const auto nbytes = def.operand_bytes.at(0);
  ins.EncodeOperand(offset, nbytes, operand);
  return ins;
}

Instruction Encode(Opcode op, const std::vector<int>& operands) {
  const auto def = LookupDefinition(op);
  CHECK_EQ(def.NumOperands(), operands.size()) << def;
  const auto total_bytes = def.SumOperandBytes() + 1;

  Instruction ins;
  auto offset = ins.EncodeOpcode(op, total_bytes);

  for (size_t i = 0; i < operands.size(); ++i) {
    const auto nbytes = def.operand_bytes[i];
    ins.EncodeOperand(offset, nbytes, operands[i]);
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
      case 1:
        dec.operands.push_back(ins.ByteAt(dec.nbytes + offset));
        break;
      case 2:
        dec.operands.push_back(ReadUint16(ins.BytePtr(dec.nbytes + offset)));
        break;
      default:
        CHECK(false) << "Should not reach here";
    }
    dec.nbytes += nbytes;
  }

  return dec;
}

}  // namespace monkey
