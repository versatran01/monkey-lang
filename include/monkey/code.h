#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace monkey {

using Byte = unsigned char;
enum class Opcode : Byte {
  Const,
};

inline constexpr Byte ToByte(Opcode op) noexcept {
  return static_cast<Byte>(op);
}

using Bytes = std::vector<Byte>;

struct Instruction {
  Instruction() = default;
  Instruction(Opcode op, const std::vector<int>& operands);

  Bytes bytes;

  auto size() const noexcept { return bytes.size(); }
  auto empty() const noexcept { return bytes.empty(); }
  std::string String() const;
  friend std::ostream& operator<<(std::ostream& os, const Instruction& inst);
};

using InstructionVec = std::vector<Instruction>;

struct Definition {
  std::string name;
  std::vector<int> widths;
};

Definition LookupDefinition(Opcode op);

Instruction ConcatInstructions(const std::vector<Instruction>& inst_vec);

}  // namespace monkey
