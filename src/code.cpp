#include "monkey/code.h"

#include <absl/container/flat_hash_map.h>

namespace monkey {

namespace {

const absl::flat_hash_map<Opcode, Definition> gOpcodeDefinitions = {
    {Opcode::kConst, {"OpConst", {2}}},
    {Opcode::kPop, {"OpPop"}},
    {Opcode::kAdd, {"OpAdd"}},
    {Opcode::kSub, {"OpSub"}},
    {Opcode::kMul, {"OpMul"}},
    {Opcode::kDiv, {"OpDiv"}},
    {Opcode::kTrue, {"OpTrue"}},
    {Opcode::kFalse, {"OpFalse"}},
    {Opcode::kEq, {"OpEq"}},
    {Opcode::kNe, {"OpNe"}},
    {Opcode::kGt, {"OpGt"}},
    {Opcode::kMinus, {"OpMinus"}},
    {Opcode::kBang, {"OpBang"}},
    {Opcode::kJumpNotTrue, {"OpJumpNotTrue", {2}}},
    {Opcode::kJump, {"OpJump", {2}}},
    {Opcode::kNull, {"OpNull"}},
    {Opcode::kGetGlobal, {"OpGetGlobal", {2}}},
    {Opcode::kSetGlobal, {"OpSetGlobal", {2}}},
};

}  // namespace

std::string Repr(Opcode op) { return gOpcodeDefinitions.at(op).name; }
std::ostream& operator<<(std::ostream& os, Opcode op) { return os << Repr(op); }

Definition LookupDefinition(Opcode op) { return gOpcodeDefinitions.at(op); }

size_t Definition::SumOperandBytes() const {
  return std::accumulate(
      operand_bytes.cbegin(), operand_bytes.cend(), size_t{0});
}

}  // namespace monkey
