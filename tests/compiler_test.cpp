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
  const auto bc = compiler.Compile(program);
  ASSERT_TRUE(bc.ok()) << bc.status();
  // Check instructions
  const auto expected = ConcatInstructions(test.inst_vec);
  //  EXPECT_EQ(bc->ins, expected);
  EXPECT_EQ(bc->ins.Repr(), expected.Repr());
  EXPECT_THAT(bc->consts, ContainerEq(test.constants));
}

TEST(CompilerTest, TestIntArithmetic) {
  const std::vector<CompilerTest> tests = {
      {"1 + 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kAdd),
        Encode(Opcode::kPop)}},
      {"1; 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kPop),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kPop)}},
      {"1 - 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kSub),
        Encode(Opcode::kPop)}},
      {"1 * 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kMul),
        Encode(Opcode::kPop)}},
      {"2 / 1",
       {IntObj(2), IntObj(1)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kDiv),
        Encode(Opcode::kPop)}},
      {"-1",
       {IntObj(1)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kMinus),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestBooleanExpression) {
  const std::vector<CompilerTest> tests = {
      {"true", {}, {Encode(Opcode::kTrue), Encode(Opcode::kPop)}},
      {"false", {}, {Encode(Opcode::kFalse), Encode(Opcode::kPop)}},
      {"1 > 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kGt),
        Encode(Opcode::kPop)}},
      {"1 < 2",
       {IntObj(2), IntObj(1)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kGt),
        Encode(Opcode::kPop)}},
      {"1 == 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kEq),
        Encode(Opcode::kPop)}},
      {"1 != 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, {0}),
        Encode(Opcode::kConst, {1}),
        Encode(Opcode::kNe),
        Encode(Opcode::kPop)}},
      {"true == false",
       {},
       {Encode(Opcode::kTrue),
        Encode(Opcode::kFalse),
        Encode(Opcode::kEq),
        Encode(Opcode::kPop)}},
      {"true != false",
       {},
       {Encode(Opcode::kTrue),
        Encode(Opcode::kFalse),
        Encode(Opcode::kNe),
        Encode(Opcode::kPop)}},
      {"!true",
       {},
       {Encode(Opcode::kTrue), Encode(Opcode::kBang), Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestConditional) {
  const std::vector<CompilerTest> tests = {
      {"if (true) { 10 }; 3333;",
       {IntObj(10), IntObj(3333)},
       {
           // 0000
           Encode(Opcode::kTrue),
           // 0001
           Encode(Opcode::kJumpNotTrue, {10}),
           // 0004
           Encode(Opcode::kConst, {0}),
           // 0007
           Encode(Opcode::kJump, {11}),
           // 0010
           Encode(Opcode::kNull),
           // 0011
           Encode(Opcode::kPop),
           // 0012
           Encode(Opcode::kConst, {1}),
           // 0015
           Encode(Opcode::kPop),
       }},
      {"if (true) { 10 } else { 20 }; 3333;",
       {IntObj(10), IntObj(20), IntObj(3333)},
       {
           // 0000
           Encode(Opcode::kTrue),
           // 0001
           Encode(Opcode::kJumpNotTrue, {10}),
           // 0004
           Encode(Opcode::kConst, {0}),
           // 0007
           Encode(Opcode::kJump, {13}),
           // 0010
           Encode(Opcode::kConst, {1}),
           // 0013
           Encode(Opcode::kPop),
           // 0014
           Encode(Opcode::kConst, {2}),
           // 0017
           Encode(Opcode::kPop),
       }},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

}  // namespace
