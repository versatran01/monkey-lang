#include "monkey/code.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/ostream.h>

namespace monkey {

namespace {

const absl::flat_hash_map<Opcode, Definition> gOpcodeDefinitions = {
    {Opcode::kConst, {"OpConst", {2}}},
    {Opcode::kPop, {"OpPop"}},
    {Opcode::kTrue, {"OpTrue"}},
    {Opcode::kFalse, {"OpFalse"}},
    {Opcode::kAdd, {"OpAdd"}},
    {Opcode::kSub, {"OpSub"}},
    {Opcode::kMul, {"OpMul"}},
    {Opcode::kDiv, {"OpDiv"}},
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
    {Opcode::kArray, {"OpArray", {2}}},
    {Opcode::kDict, {"OpDict", {2}}},
    {Opcode::kIndex, {"OpIndex"}},
    {Opcode::kCall, {"OpCall", {1}}},
    {Opcode::kReturn, {"OpReturn"}},
    {Opcode::kReturnVal, {"OpReturnVal"}},
    {Opcode::kGetLocal, {"OpGetLocal", {1}}},
    {Opcode::kSetLocal, {"OpSetLocal", {1}}},
    {Opcode::kGetBuiltin, {"OpGetBuiltin", {1}}},
};

}  // namespace

std::string Repr(Opcode op) { return gOpcodeDefinitions.at(op).name; }
std::ostream& operator<<(std::ostream& os, Opcode op) { return os << Repr(op); }

Definition LookupDefinition(Opcode op) { return gOpcodeDefinitions.at(op); }

std::string Definition::Repr() const {
  return fmt::format(
      "Def(op={}, operands=[{}])", name, absl::StrJoin(operand_bytes, ", "));
}

std::ostream& operator<<(std::ostream& os, const Definition& def) {
  return os << def.Repr();
}

size_t Definition::SumOperandBytes() const {
  return std::accumulate(
      operand_bytes.cbegin(), operand_bytes.cend(), size_t{0});
}

}  // namespace monkey
