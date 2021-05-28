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

TEST(CompilerTest, TestSymbolDefine) {
  const SymbolDict tests = {
      {"a", {"a", kGlobalScope, 0}},
      {"b", {"b", kGlobalScope, 1}},
  };

  SymbolTable global;
  auto a = global.Define("a");
  EXPECT_EQ(a, tests.at("a"));
  EXPECT_EQ(global.NumDefs(), 1);

  auto b = global.Define("b");
  EXPECT_EQ(b, tests.at("b"));
  EXPECT_EQ(global.NumDefs(), 2);
}

TEST(CompilerTest, TestSymbolResolve) {
  const std::vector<Symbol> tests = {
      {"a", kGlobalScope, 0},
      {"b", kGlobalScope, 1},
  };

  SymbolTable global;
  global.Define("a");
  global.Define("b");
  EXPECT_EQ(global.NumDefs(), 2);

  auto a = global.Resolve("a");
  ASSERT_TRUE(a.has_value());
  EXPECT_EQ(*a, tests[0]);

  auto b = global.Resolve("b");
  ASSERT_TRUE(b.has_value());
  EXPECT_EQ(*b, tests[1]);

  auto c = global.Resolve("c");
  EXPECT_FALSE(c.has_value());
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
       {CompiledObj(Encode(Opcode::kReturn))},
       {Encode(Opcode::kConst, 0), Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

TEST(CompilerTest, TestCompilerScope) {
  Compiler comp;
  ASSERT_EQ(comp.NumScopes(), 1);

  comp.Emit(Opcode::kMul);
  comp.EnterScope();
  ASSERT_EQ(comp.NumScopes(), 2);

  comp.Emit(Opcode::kSub);
  EXPECT_EQ(comp.ScopedIns().NumBytes(), 1);
  EXPECT_EQ(comp.ScopedLast().op, Opcode::kSub);

  comp.ExitScope();
  ASSERT_EQ(comp.NumScopes(), 1);

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
        Encode(Opcode::kCall),
        Encode(Opcode::kPop)}},
      {"let noArg = fn() { 24 }; noArg();",
       {IntObj(24),
        CompiledObj({Encode(Opcode::kConst, 0), Encode(Opcode::kReturnVal)})},
       {Encode(Opcode::kConst, 1),
        Encode(Opcode::kSetGlobal, 0),
        Encode(Opcode::kGetGlobal, 0),
        Encode(Opcode::kCall),
        Encode(Opcode::kPop)}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckCompiler(test);
  }
}

}  // namespace
