#pragma once

#include "monkey/code.h"

namespace monkey {

struct Instruction {
  Bytes bytes;
  size_t num_ops{0};

  auto NumOps() const noexcept { return num_ops; }
  auto NumBytes() const noexcept { return bytes.size(); }
  void Append(const Instruction& ins);
  Byte PopBack();

  /// Encode opcode and also allocate enough space for operands
  size_t EncodeOpcode(Opcode op, size_t total_bytes = 1);
  void EncodeOperand(size_t offset, size_t nbytes, int operand);

  std::string Repr() const;
  friend std::ostream& operator<<(std::ostream& os, const Instruction& ins);

  friend bool operator==(const Instruction& lhs, const Instruction& rhs) {
    return lhs.NumOps() == rhs.NumOps() && lhs.NumBytes() == rhs.NumBytes() &&
           std::equal(lhs.bytes.begin(),
                      lhs.bytes.end(),
                      rhs.bytes.begin(),
                      rhs.bytes.end());
  }

  friend bool operator!=(const Instruction& lhs, const Instruction& rhs) {
    return !(lhs == rhs);
  }

  Instruction& operator+=(const Instruction& ins) noexcept {
    Append(ins);
    return *this;
  }

  friend Instruction operator+(Instruction lhs,
                               const Instruction& rhs) noexcept {
    return lhs += rhs;
  }
};

struct Decoded {
  std::vector<int> operands;
  size_t nbytes{0};
};

Instruction Encode(Opcode op, int operand);
Instruction Encode(Opcode op, const std::vector<int>& operands = {});
Decoded Decode(const Definition& def,
               const Instruction& ins,
               size_t offset = 0);

Instruction ConcatInstructions(const std::vector<Instruction>& instrs);

}  // namespace monkey
