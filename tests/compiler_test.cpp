#include "monkey/compiler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace {

using namespace monkey;
using ::testing::ContainerEq;

struct TestStruct {
  std::string input;
  std::vector<Object> constants;
  std::vector<Instruction> inst_vec;
};

Program Parse(const std::string& input) {
  Parser parser{input};
  return parser.ParseProgram();
}

Instruction ConcatInstructions(const std::vector<Instruction>& inst_vec) {
  Instruction out;
  for (const auto& inst : inst_vec) {
    out.insert(out.end(), inst.begin(), inst.end());
  }
  return out;
}

void CheckCompiler(const TestStruct& test) {
  const auto program = Parse(test.input);

  Compiler compiler;
  const auto bytecode = compiler.Compile(program);
  // Check instructions
  const auto expected = ConcatInstructions(test.inst_vec);
  EXPECT_THAT(bytecode.intstruction, ContainerEq(expected));

  EXPECT_THAT(bytecode.constants, ContainerEq(test.constants));
}

TEST(CompilerTest, TestIntArithmetic) {
  const std::vector<TestStruct> tests = {
      {"1 + 2",
       {IntObj(1), IntObj(2)},
       {MakeInstruction(Opcode::Const, {0}),
        MakeInstruction(Opcode::Const, {1})}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

}  // namespace
