#include "monkey/evaluator.h"

#include <absl/types/variant.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace {

using namespace monkey;
using namespace std::string_literals;

using LiteralType = absl::variant<void*, bool, int, std::string>;

struct EvalTest {
  std::string input;
  LiteralType value;
};

Object ParseAndEval(const std::string& input) {
  Parser parser{input};
  const auto program = parser.ParseProgram();
  Evaluator eval;
  Environment env;
  return eval.Evaluate(program, env);
}

void CheckLiteral(const Object& obj, const LiteralType& value) {
  switch (value.index()) {
    case 0:
      EXPECT_EQ(obj.Type(), ObjectType::kNull);
      break;
    case 1: {
      EXPECT_EQ(obj, BoolObj(std::get<1>(value)));
      break;
    }
    case 2: {
      EXPECT_EQ(obj, IntObj(std::get<2>(value)));
      break;
    }
    case 3: {  // str
      const auto& str = std::get<3>(value);
      if (IsObjError(obj)) {
        EXPECT_EQ(obj.Inspect(), str);
      } else {
        ASSERT_EQ(obj.Type(), ObjectType::kStr);
        EXPECT_EQ(obj.Cast<std::string>(), str);
      }
      break;
    }
    default:
      ASSERT_FALSE(true) << "Should not reach here";
  }
}

TEST(EvaluatorTest, TestEvalIntergerExpression) {
  const std::vector<EvalTest> tests = {
      {"5", 5},
      {"10", 10},
      {"-5", -5},
      {"-10", -10},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"-50 + 100 + -50", 0},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"20 + 2 * -10", 0},
      {"50 / 2 * 2 + 10", 60},
      {"2 * (5 + 10)", 30},
      {"3 * 3 * 3 + 10", 37},
      {"3 * (3 * 3) + 10", 37},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestEvalBooleanExpression) {
  const std::vector<EvalTest> tests = {
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
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestBangOperator) {
  const std::vector<EvalTest> tests = {
      {"!true", false},
      {"!false", true},
      {"!5", false},
      {"!!true", true},
      {"!!false", false},
      {"!!5", true},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestIfElseExpression) {
  const std::vector<EvalTest> tests = {
      {"if (true) { 10 }", 10},
      {"if (false) { 10 }", nullptr},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 > 2) { 10 }", nullptr},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if (1 < 2) { 10 } else { 20 }", 10},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestReturnStatements) {
  const std::vector<EvalTest> tests = {
      {"return 10;", 10},
      {"return 10; 9;", 10},
      {"return 2 * 5; 9;", 10},
      {"9; return 2 * 5; 9;", 10},
      {"if (10 > 1) { if (10 > 1) { return 10; } return 1;", 10}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestErrorHandling) {
  const std::vector<EvalTest> tests = {
      {"5 + true;", "type mismatch: INT + BOOL"s},
      {"5 + true; 5;", "type mismatch: INT + BOOL"s},
      {"-true", "unknown operator: -BOOL"s},
      {"true + false;", "unknown operator: BOOL + BOOL"s},
      {"5; true + false; 5", "unknown operator: BOOL + BOOL"s},
      {"if (10 > 1) { true + false; }", "unknown operator: BOOL + BOOL"s},
      {"if (true > 1) { true + 1; }", "type mismatch: BOOL > INT"s},
      {"return true + 1;", "type mismatch: BOOL + INT"s},
      {"if (10 > 1) { if (10 > 1) { return true + false; } return 1; }",
       "unknown operator: BOOL + BOOL"s},
      {"foobar", "identifier not found: foobar"s},
      {R"r("Hello" - "World" )r", "unknown operator: STR - STR"s},
      {R"r({"name": "x"}[fn(x) { x }];)r", "unusable as dict key: FUNC"s}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestLetStatement) {
  const std::vector<EvalTest> tests = {
      {"let a = 5; a;", 5},
      {"let a = 5 * 5; a;", 25},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestFunctionObject) {
  const std::string input = "fn(x) { x + 2; return 3; };";
  const auto obj = ParseAndEval(input);
  ASSERT_EQ(obj.Type(), ObjectType::kFunc);
  const auto& fobj = obj.Cast<FuncObject>();
  ASSERT_EQ(fobj.params.size(), 1);
  EXPECT_EQ(fobj.params.front().String(), "x");
  EXPECT_EQ(fobj.body.String(), "(x + 2); return 3;");
}

TEST(EvaluatorTest, TestFunctionApplication) {
  const std::vector<EvalTest> tests = {
      {"let identity = fn(x) { x; }; identity(5);", 5},
      {"let identity = fn(x) { return x; }; identity(5);", 5},
      {"let double = fn(x) { x * 2; }; double(5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
      {"fn(x) { x; }(5)", 5},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestClosures) {
  const std::string input = R"raw(
    let newAdder = fn(x) {
        fn(y) { x + y };
    };
    let addTwo = newAdder(2);
    addTwo(2);)raw";

  SCOPED_TRACE(input);
  const auto obj = ParseAndEval(input);
  CheckLiteral(obj, 4);
}

TEST(EvaluatorTest, TestStringLiteral) {
  const std::string input = R"raw("Hello World!")raw";
  const auto obj = ParseAndEval(input);
  SCOPED_TRACE(input);
  CheckLiteral(obj, "Hello World!"s);
}

TEST(EvaluatorTest, TestStringConcat) {
  const std::string input = R"raw("Hello" + " " + "World!")raw";
  SCOPED_TRACE(input);
  const auto obj = ParseAndEval(input);
  CheckLiteral(obj, "Hello World!"s);
}

TEST(EvaluatorTest, TestBuiltinFunctions) {
  const std::vector<EvalTest> tests = {
      {R"r(len(""))r", 0},
      {R"r(len("four"))r", 4},
      {R"r(len("hello world"))r", 11},
      {R"r(len(1))r", "argument to `len` not supported, got INT"s},
      {R"r(len("one", "two"))r", "wrong number of arguments. got=2, want=1"s}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestArrayLiterals) {
  const std::string input = "[1, 2 * 2, 3 + 3]";
  const auto obj = ParseAndEval(input);
  SCOPED_TRACE(input);
  ASSERT_EQ(obj.Type(), ObjectType::kArray);
  const auto& array = obj.Cast<Array>();
  ASSERT_EQ(array.size(), 3);
  CheckLiteral(array[0], 1);
  CheckLiteral(array[1], 2 * 2);
  CheckLiteral(array[2], 3 + 3);
}

TEST(EvaluatorTest, TestArrayIndexExpression) {
  const std::vector<EvalTest> tests = {
      {"[1, 2, 3][0]", 1},
      {"[1, 2, 3][1]", 2},
      {"[1, 2, 3][2]", 3},
      {"let i = 0; [1][i];", 1},
      {"[1, 2, 3][1 + 1];", 3},
      {"let myArray = [1, 2, 3]; myArray[2];", 3},
      {"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6},
      {"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", 2},
      {"[1, 2, 3][3]", nullptr},
      {"[1, 2, 3][-1]", nullptr},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestDictLiteral) {
  const std::string input = R"r(
    let two = "two";
    {
    "one": 10 - 9,
    two: 1 + 1,
    "thr" + "ee": 6 / 2,
    4: 4,
    true: 5,
    false: 6
    }
    )r";

  const absl::flat_hash_map<Object, IntType> expected = {
      {StrObj("one"), 1},
      {StrObj("two"), 2},
      {StrObj("three"), 3},
      {IntObj(4), 4},
      {BoolObj(true), 5},
      {BoolObj(false), 6},
  };

  const auto obj = ParseAndEval(input);
  ASSERT_EQ(obj.Type(), ObjectType::kDict);
  const auto& dict = obj.Cast<Dict>();
  ASSERT_EQ(dict.size(), expected.size());
  for (const auto& [k, v] : expected) {
    SCOPED_TRACE(k.Inspect());
    const auto it = dict.find(k);
    ASSERT_NE(it, dict.end());
    const auto& io = it->second;
    ASSERT_EQ(io.Type(), ObjectType::kInt);
    EXPECT_EQ(io.Cast<IntType>(), v);
  }
}

TEST(EvaluatorTest, TestDictIndexExpression) {
  const std::vector<EvalTest> tests = {
      {R"r({"foo": 5}["foo"])r", 5},
      {R"r({"foo": 5}["bar"])r", nullptr},
      {R"r(let key = "foo"; {"foo": 5}[key])r", 5},
      {R"r(){}["foo"])r", nullptr},
      {"{5: 5}[5]", 5},
      {"{true: 5}[true]", 5},
      {"{false: 5}[false]", 5},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

TEST(EvaluatorTest, TestFibonacci) {
  const std::string fib_code = R"r(
    let fibonacci = fn(x) {
        if (x == 0) {
            return 0;
        } else {
            if (x == 1) {
                return 1;
            } else {
                return fibonacci(x - 1) + fibonacci(x - 2);
            }
        }
    };)r";
  const std::vector<EvalTest> tests = {
      {fib_code + "fibonacci(0);", 0}, {fib_code + "fibonacci(1);", 1},
      //      {fib_code + "fibonacci(2);", 1},
      //      {fib_code + "fibonacci(3);", 2},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    const auto obj = ParseAndEval(test.input);
    CheckLiteral(obj, test.value);
  }
}

// TEST(EvaluatorTest, TestQuote) {
//  const std::vector<EvalTest> tests = {
//      {"quote(5)", "5"},
//      {"quote(5 + 8)", "(5 + 8)"},
//      {"quote(foobar)", "foobar"},
//      {"quote(foobar + barfoo)", "(foobar + barfoo)"},
//  };

//  for (const auto& test : tests) {
//    SCOPED_TRACE(test.input);
//    const auto obj = ParseAndEval(test.input);
//    ASSERT_EQ(obj.Type(), ObjectType::kQuote);
//    const auto& expr = obj.Cast<ExprNode>();
//    EXPECT_EQ(expr.String(), test.value);
//  }
//}

// TEST(EvaluatorTest, TestUnquote) {
//  const std::vector<InputExpected<std::string>> tests = {
//      {"quote(unquote(4))", "4"},
//      {"quote(unquote(4 + 4))", "8"},
//      {"quote(8 + unquote(4 + 4))", "(8 + 8)"},
//      {"quote(unquote(4 + 4) + 8)", "(8 + 8)"},
//  };

//  for (const auto& test : tests) {
//    SCOPED_TRACE(test.first);
//    const auto obj = ParseAndEval(test.first);
//    ASSERT_EQ(obj.Type(), ObjectType::kQuote);
//    const auto& expr = obj.Cast<ExprNode>();
//    EXPECT_EQ(expr.String(), test.second);
//  }
//}

}  // namespace
