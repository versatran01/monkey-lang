#include "monkey/compiler.h"

#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace {

using namespace monkey;

struct TestStruct {
  std::string input;
  std::vector<int> constants;
  std::vector<ByteVec> instructions;
};

void CheckCompiler(const TestStruct& test) {
  Parser parser{test.input};
  const auto program = parser.ParseProgram();

  Compiler compiler;
  const auto bytecode = compiler.Compile(program);
}

TEST(CompilerTest, TestIntArithmetic) {
  const std::vector<TestStruct> tests = {
      {"1 + 2",
       {1, 2},
       {MakeInstruction(Opcode::Const, {0}),
        MakeInstruction(Opcode::Const, {1})}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

}  // namespace
