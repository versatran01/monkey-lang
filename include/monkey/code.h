#pragma once

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

using ByteVec = std::vector<Byte>;

struct Definition {
  std::string name;
  std::vector<int> widths;
};

Definition LookupDefinition(Opcode op);

ByteVec MakeInstruction(Opcode op, const std::vector<int>& operands);

}  // namespace monkey
