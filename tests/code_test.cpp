#include "monkey/code.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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
  };

  for (const auto& test : tests) {
    const auto ins = Encode(test.op, test.operands);
    EXPECT_THAT(ins.bytes, ContainerEq(test.expected));
  }
}

TEST(CodeTest, TestDecode) {
  struct DecodeTest {
    Opcode op;
    std::vector<int> operands;
    int nbytes;
  };

  const std::vector<DecodeTest> tests = {
      {Opcode::kConst, {65535}, 2},
      {Opcode::kAdd, {}, 0},
  };

  for (const auto& test : tests) {
    const auto inst = Encode(test.op, test.operands);
    const auto def = LookupDefinition(test.op);
    const auto dec = Decode(def, inst, 1);
    EXPECT_EQ(dec.nbytes, test.nbytes);
    EXPECT_THAT(dec.operands, ContainerEq(test.operands));
  }
}

TEST(CodeTest, TestInstructionString) {
  const std::vector<Instruction> instrs = {
      Encode(Opcode::kAdd),
      Encode(Opcode::kConst, {2}),
      Encode(Opcode::kConst, {65534}),
  };

  const std::vector<std::string> expected = {
      "0000 OpAdd", "0000 OpConst 2", "0000 OpConst 65534"};

  for (size_t i = 0; i < instrs.size(); ++i) {
    EXPECT_EQ(instrs[i].Repr(), expected[i]);
  }

  const std::string fullstr = "0000 OpAdd\n0001 OpConst 2\n0004 OpConst 65534";

  const auto instr = ConcatInstructions(instrs);
  EXPECT_EQ(instr.Repr(), fullstr);
}

}  // namespace
