#include "monkey/compiler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace {

using namespace monkey;
using ::testing::ContainerEq;

struct CompilerTest {
  std::string input;
  std::vector<Object> constants;
  std::vector<Instruction> inst_vec;
};

Program Parse(const std::string& input) {
  Parser parser{input};
  return parser.ParseProgram();
}

void CheckCompiler(const CompilerTest& test) {
  const auto program = Parse(test.input);

  Compiler compiler;
  const auto bytecode = compiler.Compile(program);
  // Check instructions
  const auto expected = ConcatInstructions(test.inst_vec);
  EXPECT_EQ(bytecode.inst, expected);
  EXPECT_THAT(bytecode.inst.bytes, ContainerEq(expected.bytes));
  EXPECT_THAT(bytecode.consts, ContainerEq(test.constants));
}

TEST(CompilerTest, TestIntArithmetic) {
  const std::vector<CompilerTest> tests = {
      {"1 + 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::Const, {0}), Encode(Opcode::Const, {1})}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

}  // namespace
