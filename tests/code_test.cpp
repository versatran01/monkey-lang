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
    const auto inst = MakeInstruction(test.op, test.operands);
    ASSERT_EQ(inst.size(), test.expected.size());
    EXPECT_THAT(inst, ContainerEq(test.expected));
  }
}

}  // namespace
