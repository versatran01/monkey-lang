#include "monkey/code.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
using namespace monkey;

using ::testing::ContainerEq;

TEST(CodeTest, TestMake) {
  struct TestStruct {
    Opcode op;
    std::vector<int> operands;
    Bytes expected;
  };

  const std::vector<TestStruct> tests = {
      {Opcode::Const, {65534}, {ToByte(Opcode::Const), 255, 254}},
  };

  for (const auto& test : tests) {
    const auto inst = Instruction(test.op, test.operands);
    ASSERT_EQ(inst.size(), test.expected.size());
    EXPECT_THAT(inst.bytes, ContainerEq(test.expected));
  }
}

TEST(CodeTest, TestInstructionString) {
  const std::vector<Instruction> insts = {
      Instruction(Opcode::Const, {1}),
      Instruction(Opcode::Const, {2}),
      Instruction(Opcode::Const, {65535}),
  };

  const std::vector<std::string> expected = {
      "0000 OpConst 1", "0003 OpConst 2", "0006 OpConst 65535"};

  for (size_t i = 0; i < insts.size(); ++i) {
    //    EXPECT_EQ(ToString(insts[i]), expected[i]);
  }
}

}  // namespace
