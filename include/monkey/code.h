#pragma once

#include <absl/types/span.h>

#include <iosfwd>
#include <string>
#include <vector>

namespace monkey {

using Byte = unsigned char;

enum class Opcode : Byte {
  Const,
};

/// Convesion between Byte and Opcode
inline constexpr Byte ToByte(Opcode op) noexcept {
  return static_cast<Byte>(op);
}
inline constexpr Opcode ToOpcode(Byte bt) noexcept {
  return static_cast<Opcode>(bt);
}

using Bytes = std::vector<Byte>;

struct Instruction {
  Bytes bytes;

  auto size() const noexcept { return bytes.size(); }
  auto empty() const noexcept { return bytes.empty(); }
  std::string String() const;
  friend std::ostream& operator<<(std::ostream& os, const Instruction& ins);

  friend bool operator==(const Instruction& lhs, const Instruction& rhs) {
    return std::equal(
        lhs.bytes.begin(), lhs.bytes.end(), rhs.bytes.begin(), rhs.bytes.end());
  }

  friend bool operator!=(const Instruction& lhs, const Instruction& rhs) {
    return !(lhs == rhs);
  }
};

using InstructionVec = std::vector<Instruction>;

struct Definition {
  std::string name;
  std::vector<int> operand_bytes;

  auto NumOperands() const noexcept { return operand_bytes.size(); }
  size_t SumOperandBytes() const;
};

Definition LookupDefinition(Opcode op);

Instruction ConcatInstructions(const std::vector<Instruction>& instrs);

struct Decoded {
  std::vector<int> operands;
  int nbytes{0};
};

Instruction Encode(Opcode op, const std::vector<int>& operands);
Decoded Decode(const Definition& def, const Instruction& ins, int offset = 0);

}  // namespace monkey
