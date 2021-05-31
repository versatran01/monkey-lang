#include "monkey/compiler.h"

#include <glog/logging.h>
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

void CheckCompiler(const CompilerTest& test) {
  Parser parser{test.input};
  const auto program = parser.ParseProgram();
  ASSERT_TRUE(parser.Ok()) << parser.ErrorMsg();

  Compiler compiler;
  const auto bc = compiler.Compile(program);
  ASSERT_TRUE(bc.ok()) << bc.status();

  // Check instructions
  const auto ins = ConcatInstructions(test.inst_vec);
  EXPECT_EQ(bc->ins.Repr(), ins.Repr());
  EXPECT_THAT(bc->consts, ContainerEq(test.constants));
}

TEST(CompilerTest, TestIntArithmetic) {
  const std::vector<CompilerTest> tests = {
      {"1 + 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kAdd),
        Encode(Opcode::kPop)}},
      {"1; 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kPop),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kPop)}},
      {"1 - 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kSub),
        Encode(Opcode::kPop)}},
      {"1 * 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kMul),
        Encode(Opcode::kPop)}},
      {"2 / 1",
       {IntObj(2), IntObj(1)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kDiv),
        Encode(Opcode::kPop)}},
      {"-1",
       {IntObj(1)},
       {Encode(Opcode::kConst, 0),
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
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kGt),
        Encode(Opcode::kPop)}},
      {"1 < 2",
       {IntObj(2), IntObj(1)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kGt),
        Encode(Opcode::kPop)}},
      {"1 == 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kEq),
        Encode(Opcode::kPop)}},
      {"1 != 2",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
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
       {// 0000
        Encode(Opcode::kTrue),
        // 0001
        Encode(Opcode::kJumpNotTrue, 10),
        // 0004
        Encode(Opcode::kConst, 0),
        // 0007
        Encode(Opcode::kJump, 11),
        // 0010
        Encode(Opcode::kNull),
        // 0011
        Encode(Opcode::kPop),
        // 0012
        Encode(Opcode::kConst, 1),
        // 0015
        Encode(Opcode::kPop)}},
      {"if (true) { 10 } else { 20 }; 3333;",
       {IntObj(10), IntObj(20), IntObj(3333)},
       {// 0000
        Encode(Opcode::kTrue),
        // 0001
        Encode(Opcode::kJumpNotTrue, 10),
        // 0004
        Encode(Opcode::kConst, 0),
        // 0007
        Encode(Opcode::kJump, 13),
        // 0010
        Encode(Opcode::kConst, 1),
        // 0013
        Encode(Opcode::kPop),
        // 0014
        Encode(Opcode::kConst, 2),
        // 0017
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestGlobalLetStatement) {
  const std::vector<CompilerTest> tests = {
      {"let one = 1; let two = 2;",
       {IntObj(1), IntObj(2)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kSetGlobal, 1)}},
      {"let one = 1; one;",
       {IntObj(1)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kGetGlobal, 0),
        Encode(Opcode::kPop)}},
      {"let one = 1; let two = one; two;",
       {IntObj(1)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kGetGlobal, 0),
        Encode(Opcode::kSetGlobal, 1),
        Encode(Opcode::kGetGlobal, 1),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestStringExpression) {
  const std::vector<CompilerTest> tests = {
      {R"r("monkey")r",
       {StrObj("monkey")},
       {Encode(Opcode::kConst, 0), Encode(Opcode::kPop)}},
      {R"r("mon" + "key")r",
       {StrObj("mon"), StrObj("key")},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kAdd),
        Encode(Opcode::kPop)}}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestArrayLiteral) {
  const std::vector<CompilerTest> tests = {
      {"[]", {}, {Encode(Opcode::kArray, 0), Encode(Opcode::kPop)}},
      {"[1, 2, 3]",
       {IntObj(1), IntObj(2), IntObj(3)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kArray, 3),
        Encode(Opcode::kPop)}},
      {"[1 + 2, 3 - 4, 5 * 6]",
       {IntObj(1), IntObj(2), IntObj(3), IntObj(4), IntObj(5), IntObj(6)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kAdd),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kConst, 3),
        Encode(Opcode::kSub),
        Encode(Opcode::kConst, 4),
        Encode(Opcode::kConst, 5),
        Encode(Opcode::kMul),
        Encode(Opcode::kArray, 3),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestDictLiterals) {
  const std::vector<CompilerTest> tests = {
      {"{}", {}, {Encode(Opcode::kDict, 0), Encode(Opcode::kPop)}},
      {"{1: 2, 3: 4, 5: 6}",
       {IntObj(1), IntObj(2), IntObj(3), IntObj(4), IntObj(5), IntObj(6)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kConst, 3),
        Encode(Opcode::kConst, 4),
        Encode(Opcode::kConst, 5),
        Encode(Opcode::kDict, 6),
        Encode(Opcode::kPop)}},
      {"{1: 2 + 3, 4: 5 * 6}",
       {IntObj(1), IntObj(2), IntObj(3), IntObj(4), IntObj(5), IntObj(6)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kAdd),
        Encode(Opcode::kConst, 3),
        Encode(Opcode::kConst, 4),
        Encode(Opcode::kConst, 5),
        Encode(Opcode::kMul),
        Encode(Opcode::kDict, 4),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestIndexExpression) {
  const std::vector<CompilerTest> tests = {
      {"[1,2,3][1 + 1]",
       {IntObj(1), IntObj(2), IntObj(3), IntObj(1), IntObj(1)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kArray, 3),
        Encode(Opcode::kConst, 3),
        Encode(Opcode::kConst, 4),
        Encode(Opcode::kAdd),
        Encode(Opcode::kIndex),
        Encode(Opcode::kPop)}},
      {"{1: 2}[2 - 1]",
       {IntObj(1), IntObj(2), IntObj(2), IntObj(1)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kDict, 2),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kConst, 3),
        Encode(Opcode::kSub),
        Encode(Opcode::kIndex),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestFunction) {
  const std::vector<CompilerTest> tests = {
      {"fn() { return 5 + 10; }",
       {IntObj(5),
        IntObj(10),
        CompiledObj({Encode(Opcode::kConst, 0),
                     Encode(Opcode::kConst, 1),
                     Encode(Opcode::kAdd),
                     Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 2), Encode(Opcode::kPop)}},
      {"fn() { 5 + 10; }",
       {IntObj(5),
        IntObj(10),
        CompiledObj({Encode(Opcode::kConst, 0),
                     Encode(Opcode::kConst, 1),
                     Encode(Opcode::kAdd),
                     Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 2), Encode(Opcode::kPop)}},
      {"fn() { 1; 2; }",
       {IntObj(1),
        IntObj(2),
        CompiledObj({Encode(Opcode::kConst, 0),
                     Encode(Opcode::kPop),
                     Encode(Opcode::kConst, 1),
                     Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 2), Encode(Opcode::kPop)}},
      {"fn() { }",
       {CompiledObj({Encode(Opcode::kReturn), 0})},
       {Encode(Opcode::kConst, 0), Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestCompilerScope) {
  Compiler comp;
  ASSERT_EQ(comp.NumScopes(), 0);

  comp.EnterScope();
  ASSERT_EQ(comp.NumScopes(), 1);

  comp.Emit(Opcode::kMul);
  comp.EnterScope();
  ASSERT_EQ(comp.NumScopes(), 2);

  comp.Emit(Opcode::kSub);
  EXPECT_EQ(comp.ScopedIns().NumBytes(), 1);
  EXPECT_EQ(comp.ScopedLast().op, Opcode::kSub);

  comp.ExitScope();
  ASSERT_EQ(comp.NumScopes(), 1);

  // Check for symbol table

  comp.Emit(Opcode::kAdd);
  EXPECT_EQ(comp.ScopedIns().NumBytes(), 2);
  EXPECT_EQ(comp.ScopedLast().op, Opcode::kAdd);
  EXPECT_EQ(comp.ScopedPrev().op, Opcode::kMul);
}

TEST(CompilerTest, TestFunctionCall) {
  const std::vector<CompilerTest> tests = {
      {"fn() { 24; }()",
       {IntObj(24),
        CompiledObj({Encode(Opcode::kConst, 0), Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 1),
        Encode(Opcode::kCall, 0),
        Encode(Opcode::kPop)}},
      {"let noArg = fn() { 24 }; noArg();",
       {IntObj(24),
        CompiledObj({Encode(Opcode::kConst, 0), Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 1),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kGetGlobal, 0),
        Encode(Opcode::kCall, 0),
        Encode(Opcode::kPop)}},
      {"let oneArg = fn(a) { a }; oneArg(24);",
       {CompiledObj({Encode(Opcode::kGetLocal, 0), Encode(Opcode::kReturnVal)}),
        IntObj(24)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kGetGlobal, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kCall, 1),
        Encode(Opcode::kPop)}},
      {"let manyArg = fn(a, b, c) { a; b; c}; manyArg(24, 25, 26);",
       {CompiledObj({Encode(Opcode::kGetLocal, 0),
                     Encode(Opcode::kPop),
                     Encode(Opcode::kGetLocal, 1),
                     Encode(Opcode::kPop),
                     Encode(Opcode::kGetLocal, 2),
                     Encode(Opcode::kReturnVal)}),
        IntObj(24),
        IntObj(25),
        IntObj(26)},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kGetGlobal, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kConst, 2),
        Encode(Opcode::kConst, 3),
        Encode(Opcode::kCall, 3),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestLetStatementScope) {
  const std::vector<CompilerTest> tests = {
      {"let num = 55; fn() { num }",
       {IntObj(55),
        CompiledObj(
            {Encode(Opcode::kGetGlobal, 0), Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 0),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kConst, 1),
        Encode(Opcode::kPop)}},
      {"fn() { let num = 55; num }",
       {IntObj(55),
        CompiledObj({Encode(Opcode::kConst, 0),
                     Encode(Opcode::kSetLocal, 0),
                     Encode(Opcode::kGetLocal, 0),
                     Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 1), Encode(Opcode::kPop)}},
      {"fn() { let a = 55; let b = 77; a + b }",
       {IntObj(55),
        IntObj(77),
        CompiledObj({Encode(Opcode::kConst, 0),
                     Encode(Opcode::kSetLocal, 0),
                     Encode(Opcode::kConst, 1),
                     Encode(Opcode::kSetLocal, 1),
                     Encode(Opcode::kGetLocal, 0),
                     Encode(Opcode::kGetLocal, 1),
                     Encode(Opcode::kAdd),
                     Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 2), Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

}  // namespace
