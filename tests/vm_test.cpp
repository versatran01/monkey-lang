#include "monkey/vm.h"

#include <absl/types/variant.h>
#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "monkey/compiler.h"
#include "monkey/object.h"
#include "monkey/parser.h"

namespace {
using namespace monkey;
using namespace std::string_literals;

using IntVec = std::vector<int>;
using IntDict = absl::flat_hash_map<int, int>;

using LiteralType =
    absl::variant<void*, int, bool, std::string, IntVec, IntDict>;

struct VmTest {
  std::string input;
  LiteralType value;
};

MATCHER_P(MAP_MATCHER, map, "Maps are not equal") {
  if (map.size() != arg.size()) return false;

  return std::all_of(map.cbegin(), map.cend(), [&](const auto& pair) {
    auto it = arg.find(pair.first);
    if (it == arg.cend()) return false;
    return it->second == pair.second;
  });
}

Program Parse(const std::string& input) {
  Parser parser{input};
  return parser.ParseProgram();
}

void CheckVmError(const VmTest& test) {
  const auto program = Parse(test.input);
  Compiler comp;
  const auto bc = comp.Compile(program);
  ASSERT_TRUE(bc.ok());

  VirtualMachine vm;
  const auto status = vm.Run(bc.value());

  ASSERT_EQ(test.value.index(), 3);
  const std::string msg = std::get<3>(test.value);
  EXPECT_EQ(std::string{status.message()}, msg);
}

void CheckVm(const VmTest& test) {
  const auto program = Parse(test.input);
  Compiler comp;
  const auto bc = comp.Compile(program);

  ASSERT_TRUE(bc.ok()) << bc.status();
  VirtualMachine vm;
  const auto status = vm.Run(bc.value());
  ASSERT_TRUE(status.ok()) << status;

  switch (test.value.index()) {
    case 0:
      EXPECT_EQ(vm.Last(), NullObj());
      break;
    case 1:
      EXPECT_EQ(vm.Last(), IntObj(std::get<1>(test.value)));
      break;
    case 2:
      EXPECT_EQ(vm.Last(), BoolObj(std::get<2>(test.value)));
      break;
    case 3:
      EXPECT_EQ(vm.Last(), StrObj(std::get<3>(test.value)));
      break;
    case 4: {
      Array arr;
      const auto& ivec = std::get<4>(test.value);
      for (const auto& i : ivec) {
        arr.push_back(IntObj(i));
      }
      const auto obj = ArrayObj(std::move(arr));
      EXPECT_EQ(vm.Last(), obj);
      break;
    }
    case 5: {
      Dict dict;
      const auto& idict = std::get<5>(test.value);
      for (const auto& [k, v] : idict) {
        dict[IntObj(k)] = IntObj(v);
      }
      EXPECT_THAT(vm.Last().Cast<Dict>(), MAP_MATCHER(dict));
      break;
    }
    default:
      ASSERT_TRUE(false) << "Unhandeld type";
  }
}

TEST(VmTest, TestIntArithmetic) {
  const std::vector<VmTest> tests = {
      {"1", 1},
      {"2", 2},
      {"1 + 2", 3},
      {"1 - 2", -1},
      {"1 * 2", 2},
      {"4 / 2", 2},
      {"50 / 2 * 2 + 10 - 5", 55},
      {"5 * (2 + 10)", 60},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"5 * (2 + 10)", 60},
      {"-5", -5},
      {"-10", -10},
      {"-50 + 100 + -50", 0},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestBooleanExpression) {
  const std::vector<VmTest> tests = {
      {"true", true},
      {"false", false},
      {"1 < 2", true},
      {"1 > 2", false},
      {"1 < 1", false},
      {"1 > 1", false},
      {"1 == 1", true},
      {"1 != 1", false},
      {"1 == 2", false},
      {"1 != 2", true},
      {"true == true", true},
      {"false == false", true},
      {"true == false", false},
      {"true != false", true},
      {"false != true", true},
      {"(1 < 2) == true", true},
      {"(1 < 2) == false", false},
      {"(1 > 2) == true", false},
      {"(1 > 2) == false", true},
      {"!true", false},
      {"!false", true},
      {"!5", false},
      {"!!true", true},
      {"!!false", false},
      {"!!5", true},
      {"!(if (false) { 5; })", true},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestConditional) {
  const std::vector<VmTest> tests = {
      {"if (true) { 10 }", 10},
      {"if (true) { 10 } else { 20 }", 10},
      {"if (false) { 10 } else { 20 } ", 20},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 < 2) { 10 } else { 20 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if (1 > 2) { 10 }", nullptr},
      {"if (false) { 10 }", nullptr},
      {"if ((if (false) { 10 })) { 10 } else { 20 }", 20}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestGlobalLetStatement) {
  const std::vector<VmTest> tests = {
      {"let one = 1; one", 1},
      {"let one = 1; let two = 2; one + two", 3},
      {"let one = 1; let two = one + one; one + two", 3},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestStringExpression) {
  const std::vector<VmTest> tests = {
      {R"r("monkey")r", "monkey"s},
      {R"r("mon" + "key")r", "monkey"s},
      {R"r("mon" + "key" + "banana")r", "monkeybanana"s},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestArrayLiteral) {
  const std::vector<VmTest> tests = {
      {"[]", IntVec{}},
      {"[1, 2, 3]", IntVec{1, 2, 3}},
      {"[1 + 2, 3 * 4, 5 + 6]", IntVec{3, 12, 11}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestDictLiteral) {
  const std::vector<VmTest> tests = {
      {"{}", IntDict{}},
      {"{1: 2, 2: 3}", IntDict{{1, 2}, {2, 3}}},
      {"{1 + 1: 2 * 2, 3 + 3: 4 * 4}", IntDict{{2, 4}, {6, 16}}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestIndexExpression) {
  const std::vector<VmTest> tests = {
      {"[1, 2, 3][1]", 2},
      {"[1, 2, 3][0 + 2]", 3},
      {"[[1, 1, 1]][0][0]", 1},
      {"[][0]", nullptr},
      {"[1, 2, 3][99]", nullptr},
      {"[1][-1]", nullptr},
      {"{1: 1, 2: 2}[1]", 1},
      {"{1: 1, 2: 2}[2]", 2},
      {"{1: 1}[0]", nullptr},
      {"{}[0]", nullptr},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestCallFunctionNoArgs) {
  const std::vector<VmTest> tests = {
      {"let fivePlusTen = fn() { 5 + 10; }; fivePlusTen();", 15},
      {"let one = fn() { 1; }; let two = fn() { 2; }; one() + two()", 3},
      {"let a = fn(){1};let b = fn(){a() + 1}; let c = fn(){b() + 1}; c();", 3},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestCallFunctionWithReturn) {
  const std::vector<VmTest> tests = {
      {"let earlyExit = fn() { return 99; 100; }; earlyExit();", 99},
      {"let earlyExit = fn() { return 99; return 100; }; earlyExit();", 99},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestCallFunctionWithoutReturnVal) {
  const std::vector<VmTest> tests = {
      {"let noReturn = fn() { }; noReturn();", nullptr},
      {"let noReturn = fn() { }; let noReturnTwo = fn() { noReturn(); }; "
       "noReturn(); noReturnTwo();",
       nullptr},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestFirstClassFunction) {
  const std::vector<VmTest> tests = {
      {"let returnsOne = fn() { 1; }; let returnsOneReturner = fn() { "
       "returnsOne; }; returnsOneReturner()();",
       1},
      {R"r(let returnsOneReturner = fn() {
            let returnsOne = fn() { 1; };
            returnsOne;
            };
            returnsOneReturner()();)r",
       1}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestCallFunctionWithBinding) {
  const std::vector<VmTest> tests = {
      {"let one = fn() { let one = 1; one }; one();", 1},
      {"let oneAndTwo = fn() { let one = 1; let two = 2; one + two; }; "
       "oneAndTwo();",
       3},
      {"let oneAndTwo = fn() { let one = 1; let two = 2; one + two; }; let "
       "threeAndFour = fn() { let three = 3; let four = 4; three + four; }; "
       "oneAndTwo() + threeAndFour();",
       10},
      {R"r(let firstFoobar = fn() { let foobar = 50; foobar; };
            let secondFoobar = fn() { let foobar = 100; foobar; };
            firstFoobar() + secondFoobar();)r",
       150},
      {R"r(let globalSeed = 50;
        let minusOne = fn() {
            let num = 1;
            globalSeed - num;
        };
        let minusTwo = fn() {
            let num = 2;
            globalSeed - num;
        };
        minusOne() + minusTwo();)r",
       97},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestCallFunctionWithArgsAndBindings) {
  const std::vector<VmTest> tests = {
      {"let identity = fn(a) { a; }; identity(4);", 4},
      {"let sum = fn(a, b) { a + b; }; sum(1, 2);", 3},
      {"let sum = fn(a, b) { let c = a + b; c; }; sum(1, 2);", 3},
      {"let sum = fn(a, b) { let c = a + b; c; }; sum(1, 2) + sum(3, 4);", 10},
      {R"r(let sum = fn(a, b) {
            let c = a + b;
            c;
        };
        let outer = fn() {
            sum(1, 2) + sum(3, 4);
        };
        outer();)r",
       10},
      {R"r(let globalNum = 10;
        let sum = fn(a, b) {
            let c = a + b;
            c + globalNum;
        };
        let outer = fn() {
            sum(1, 2) + sum(3, 4) + globalNum;
        };
        outer() + globalNum;)r",
       50},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestCallFunctionWithWrongArguments) {
  const std::vector<VmTest> tests = {
      {"fn() { 1; }(1);", "wrong number of arguments: want=0, got=1"s},
      {"fn(a) { a; }()", "wrong number of arguments: want=1, got=0"s},
      {"fn(a, b) { a + b; }(1);", "wrong number of arguments: want=2, got=1"s},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVmError(test);
  }
}

TEST(VmTest, TestBuiltinFunctions) {
  const std::vector<VmTest> tests = {
      {R"r(len(""))r", 0},
      {R"r(len("four"))r", 4},
      {R"r(len("hello world"))r", 11},
      {"len([1,2,3])", 3},
      {"len([])", 0},
      {R"r(puts("hello", "world!")r", nullptr},
      {"first([1,2,3])", 1},
      {"first([])", nullptr},
      {"last([1,2,3])", 3},
      {"last([])", nullptr},
      {"rest([1,2,3])", IntVec{2, 3}},
      {"rest([])", nullptr},
      {"push([], 1)", IntVec{1}},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }

  const std::vector<VmTest> errors = {
      {"len(1)", "argument to `len` not supported, got INT"s},
      {R"r(len("one", "two"))r", "wrong number of arguments. got=2, want=1"s},
      {"first(1)", "argument to `first` must be ARRAY, got INT"s},
      {"last(1)", "argument to `last` must be ARRAY, got INT"s},
      {"push(1, 1)", "argument to `push` must be ARRAY, got INT"s},
  };

  for (const auto& test : errors) {
    SCOPED_TRACE(test.input);
    CheckVmError(test);
  }
}

TEST(VmTest, TestClosure) {
  const std::vector<VmTest> tests = {
      {R"r(let newClosure = fn(a) {
          fn() { a; };
        };
        let closure = newClosure(99);
        closure();)r",
       99},
      {R"r(let newAdder = fn(a, b) {
      fn(c) { a + b + c };
      };
      let adder = newAdder(1, 2);
      adder(8);)r",
       11},
      {R"r(let newAdder = fn(a, b) {
     let c = a + b;
     fn(d) { c + d };
     };
     let adder = newAdder(1, 2);
     adder(8);)r",
       11},
      {R"r(let newAdderOuter = fn(a, b) {
            let c = a + b;
            fn(d) {
            let e = d + c;
                fn(f) { e + f; };
        };
        };
     let newAdderInner = newAdderOuter(1, 2);
     let adder = newAdderInner(3);
     adder(8);)r",
       14},
      {R"r(let a = 1;
     let newAdderOuter = fn(b) {
     fn(c) {
     fn(d) { a + b + c + d };
     };
     };
     let newAdderInner = newAdderOuter(2);
     let adder = newAdderInner(3);
     adder(8);)r",
       14},
      {R"r(let newClosure = fn(a, b) {
     let one = fn() { a; };
     let two = fn() { b; };
     fn() { one() + two(); };
     };
     let closure = newClosure(9, 90);
     closure();)r",
       99},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

TEST(VmTest, TestRecursiveFibonacci) {
  const std::string fib_code = R"r(
    let fibonacci = fn(x) {
        if (x == 0) {
            0
        } else {
            if (x == 1) {
                1
            } else {
                fibonacci(x - 1) + fibonacci(x - 2);
            }
        }
    };)r";

  const std::vector<VmTest> tests = {
      {fib_code + "fibonacci(0);", 0},
      {fib_code + "fibonacci(1);", 1},
      {fib_code + "fibonacci(2);", 1},
      {fib_code + "fibonacci(3);", 2},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

}  // namespace
