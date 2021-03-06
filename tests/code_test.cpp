#include "monkey/code.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "monkey/instruction.h"

namespace {
using namespace monkey;

using ::testing::ContainerEq;

TEST(CodeTest, TestEncode) {
  struct EncodeTest {
    Opcode op;
    std::vector<int> operands;
    Bytes expected;
  };

  const std::vector<EncodeTest> tests = {
      {Opcode::kConst, {65534}, {ToByte(Opcode::kConst), 255, 254}},
      {Opcode::kAdd, {}, {ToByte(Opcode::kAdd)}},
      {Opcode::kGetLocal, {255}, {ToByte(Opcode::kGetLocal), 255}},
      {Opcode::kClosure,
       {65534, 255},
       {ToByte(Opcode::kClosure), 255, 254, 255}},
  };

  for (const auto& test : tests) {
    const auto ins = Encode(test.op, test.operands);
    EXPECT_THAT(ins.bytes, ContainerEq(test.expected));
    EXPECT_EQ(ins.NumOps(), 1);
    EXPECT_EQ(ins.NumBytes(), test.expected.size());
  }
}

TEST(CodeTest, TestEncodeSingle) {
  struct EncodeTest {
    Opcode op;
    int operand;
    Bytes expected;
  };

  const std::vector<EncodeTest> tests = {
      {Opcode::kConst, 65534, {ToByte(Opcode::kConst), 255, 254}}};

  for (const auto& test : tests) {
    const auto ins = Encode(test.op, test.operand);
    EXPECT_THAT(ins.bytes, ContainerEq(test.expected));
    EXPECT_EQ(ins.NumOps(), 1);
    EXPECT_EQ(ins.NumBytes(), test.expected.size());
  }
}  // namespace

TEST(CodeTest, TestDecode) {
  struct DecodeTest {
    Opcode op;
    std::vector<int> operands;
    size_t nbytes;
  };

  const std::vector<DecodeTest> tests = {
      {Opcode::kConst, {65535}, 2},
      {Opcode::kAdd, {}, 0},
      {Opcode::kGetLocal, {255}, 1},
  };

  for (const auto& test : tests) {
    const auto inst = Encode(test.op, test.operands);
    const auto def = LookupDefinition(test.op);
    const auto dec = Decode(def, inst, 1);
    EXPECT_EQ(dec.nbytes, test.nbytes);
    EXPECT_THAT(absl::MakeConstSpan(dec.operands),
                ContainerEq(absl::MakeConstSpan(test.operands)));
  }
}

TEST(CodeTest, TestInstructionString) {
  const std::vector<Instruction> instructions = {
      Encode(Opcode::kAdd),
      Encode(Opcode::kGetLocal, 1),
      Encode(Opcode::kConst, 2),
      Encode(Opcode::kConst, 65534),
      Encode(Opcode::kClosure, {65534, 255}),
  };

  const std::vector<std::string> expected = {
      "0000 OpAdd",
      "0000 OpGetLocal 1",
      "0000 OpConst 2",
      "0000 OpConst 65534",
      "0000 OpClosure 65534 255",
  };

  for (size_t i = 0; i < instructions.size(); ++i) {
    EXPECT_EQ(instructions[i].Repr(), expected[i]);
  }

  const std::string fullstr =
      "0000 OpAdd\n0001 OpGetLocal 1\n0003 OpConst 2\n0006 OpConst 65534\n0009 "
      "OpClosure 65534 255";

  const auto instr = ConcatInstructions(instructions);
  EXPECT_EQ(instr.Repr(), fullstr);
}

}  // namespace
