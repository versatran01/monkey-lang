#pragma once

#include <absl/container/inlined_vector.h>

#include <cstring>
#include <iosfwd>
#include <string>
#include <vector>

namespace monkey {

using Byte = unsigned char;
using Bytes = std::vector<Byte>;

enum class Opcode : Byte {
  kConst,
  kPop,
  kTrue,
  kFalse,
  kAdd,
  kSub,
  kMul,
  kDiv,
  kEq,
  kNe,
  kGt,
  kMinus,
  kBang,
  kJumpNotTrue,
  kJump,
  kNull,
  kGetGlobal,
  kSetGlobal,
  kArray,
  kDict,
  kIndex,
  kCall,
  kReturn,
  kReturnVal,
  kGetLocal,
  kSetLocal,
  kGetBuiltin,
  kClosure,
};

std::string Repr(Opcode op);
std::ostream& operator<<(std::ostream& os, Opcode op);

/// Conversion between Byte and Opcode
inline constexpr Byte ToByte(Opcode op) noexcept {
  return static_cast<Byte>(op);
}
inline constexpr Opcode ToOpcode(Byte bt) noexcept {
  return static_cast<Opcode>(bt);
}

struct Definition {
  std::string name;
  absl::InlinedVector<size_t, 2> operand_bytes{};

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Definition& def);

  size_t NumOperands() const noexcept { return operand_bytes.size(); }
  size_t SumOperandBytes() const;
};

Definition LookupDefinition(Opcode op);

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
