#pragma once

#include <absl/types/span.h>

#include <cstring>
#include <iosfwd>
#include <string>
#include <vector>

namespace monkey {

using Byte = unsigned char;

enum class Opcode : Byte {
  kConst,
  kPop,
  kAdd,
  kSub,
  kMul,
  kDiv,
  kTrue,
  kFalse,
  kEq,
  kNe,
  kGt,
  kMinus,
  kBang,
  kJumpNotTrue,
  kJump,
};

std::string Repr(Opcode op);
std::ostream& operator<<(std::ostream& os, Opcode op);

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
  size_t num_ops{0};

  auto NumOps() const noexcept { return num_ops; }
  auto NumBytes() const noexcept { return bytes.size(); }
  void Append(const Instruction& ins);

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Instruction& ins);

  friend bool operator==(const Instruction& lhs, const Instruction& rhs) {
    return std::equal(
        lhs.bytes.begin(), lhs.bytes.end(), rhs.bytes.begin(), rhs.bytes.end());
  }

  friend bool operator!=(const Instruction& lhs, const Instruction& rhs) {
    return !(lhs == rhs);
  }
};

Instruction ConcatInstructions(const std::vector<Instruction>& instrs);

struct Definition {
  std::string name;
  std::vector<size_t> operand_bytes{};

  auto NumOperands() const noexcept { return operand_bytes.size(); }
  size_t SumOperandBytes() const;
};

Definition LookupDefinition(Opcode op);

struct Decoded {
  std::vector<int> operands;
  size_t nbytes{0};
};

Instruction Encode(Opcode op, const std::vector<int>& operands = {});
Decoded Decode(const Definition& def,
               const Instruction& ins,
               size_t offset = 0);

// Helper functions
inline uint16_t SwapUint16Bytes(uint16_t n) {
  return static_cast<uint16_t>((n >> 8) | (n << 8));
}

inline void PutUint16(uint8_t* dst, uint16_t n) {
  n = SwapUint16Bytes(n);
  std::memcpy(dst, &n, sizeof(uint16_t));
}

inline uint16_t ReadUint16(const uint8_t* src) {
  uint16_t n = *reinterpret_cast<const uint16_t*>(src);
  return SwapUint16Bytes(n);
}

}  // namespace monkey
