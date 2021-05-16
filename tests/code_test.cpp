#include "monkey/code.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
using namespace monkey;

using ::testing::ContainerEq;

TEST(CodeTest, TestEncode) {
  struct Test {
    Opcode op;
    std::vector<int> operands;
    Bytes expected;
  };

  const std::vector<Test> tests = {
      {Opcode::kConst, {65534}, {ToByte(Opcode::kConst), 255, 254}},
  };

  for (const auto& test : tests) {
    const auto inst = Encode(test.op, test.operands);
    EXPECT_THAT(inst.bytes, ContainerEq(test.expected));
  }
}

TEST(CodeTest, TestDecode) {
  struct Test {
    Opcode op;
    std::vector<int> operands;
    int nbytes;
  };

  const std::vector<Test> tests = {
      {Opcode::kConst, {65535}, 2},
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
      Encode(Opcode::kConst, {1}),
      Encode(Opcode::kConst, {2}),
      Encode(Opcode::kConst, {65534}),
  };

  const std::vector<std::string> expected = {
      "0000 OpConst 1\n", "0000 OpConst 2\n", "0000 OpConst 65534\n"};

  for (size_t i = 0; i < instrs.size(); ++i) {
    EXPECT_EQ(instrs[i].String(), expected[i]);
  }

  const std::string fullstr =
      "0000 OpConst 1\n0003 OpConst 2\n0006 OpConst 65534\n";

  const auto instr = ConcatInstructions(instrs);
  EXPECT_EQ(instr.String(), fullstr);
}

}  // namespace
