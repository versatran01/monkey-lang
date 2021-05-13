#include "monkey/evaluator.h"

#include <absl/types/variant.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace monkey {
namespace {

template <typename T>
using InputExpected = std::pair<std::string, T>;

Object ParseAndEval(const std::string& input) {
  Parser parser{input};
  const auto program = parser.ParseProgram();
  Evaluator eval;
  Environment env;
  return eval.Evaluate(program, env);
}

void CheckBoolObj(const Object& obj, bool value) {
  ASSERT_EQ(obj.Type(), ObjectType::kBool) << obj.Inspect();
  EXPECT_EQ(obj.Cast<bool>(), value);
}

void CheckStrObj(const Object& obj, const std::string& value) {
  ASSERT_EQ(obj.Type(), ObjectType::kStr) << obj.Inspect();
  EXPECT_EQ(obj.Cast<std::string>(), value);
}

void CheckIntObj(const Object& obj, int64_t value) {
  ASSERT_EQ(obj.Type(), ObjectType::kInt) << obj.Inspect();
  EXPECT_EQ(obj.Cast<int64_t>(), value);
}

void CheckErrorObj(const Object& obj, const std::string& msg) {
  ASSERT_EQ(obj.Type(), ObjectType::kError) << obj.Inspect();
  EXPECT_EQ(obj.Inspect(), msg);
}

TEST(EvaluatorTest, TestEvalIntergerExpression) {
  const std::vector<InputExpected<int64_t>> tests = {
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
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckIntObj(obj, test.second);
  }
}

TEST(EvaluatorTest, TestEvalBooleanExpression) {
  const std::vector<InputExpected<bool>> tests = {
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
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckBoolObj(obj, test.second);
  }
}

TEST(EvaluatorTest, TestBangOperator) {
  const std::vector<InputExpected<bool>> tests = {
      {"!true", false},
      {"!false", true},
      {"!5", false},
      {"!!true", true},
      {"!!false", false},
      {"!!5", true},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckBoolObj(obj, test.second);
  }
}

TEST(EvaluatorTest, TestIfElseExpression) {
  using Value = absl::variant<void*, int64_t>;
  const std::vector<InputExpected<Value>> tests = {
      {"if (true) { 10 }", 10},
      {"if (false) { 10 }", nullptr},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 > 2) { 10 }", nullptr},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if (1 < 2) { 10 } else { 20 }", 10},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    switch (test.second.index()) {
      case 0:
        EXPECT_EQ(obj.Type(), ObjectType::kNull);
        break;
      case 1:
        CheckIntObj(obj, std::get<1>(test.second));
        break;
      default:
        ASSERT_FALSE(true) << "Should not reach here";
    }
  }
}

TEST(EvaluatorTest, TestReturnStatements) {
  const std::vector<InputExpected<int64_t>> tests = {
      {"return 10;", 10},
      {"return 10; 9;", 10},
      {"return 2 * 5; 9;", 10},
      {"9; return 2 * 5; 9;", 10},
      {"if (10 > 1) { if (10 > 1) { return 10; } return 1;", 10}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckIntObj(obj, test.second);
  }
}

TEST(EvaluatorTest, TestErrorHandling) {
  const std::vector<InputExpected<std::string>> tests = {
      {"5 + true;", "type mismatch: INTEGER + BOOLEAN"},
      {"5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"},
      {"-true", "unknown operator: -BOOLEAN"},
      {"true + false;", "unknown operator: BOOLEAN + BOOLEAN"},
      {"5; true + false; 5", "unknown operator: BOOLEAN + BOOLEAN"},
      {"if (10 > 1) { true + false; }", "unknown operator: BOOLEAN + BOOLEAN"},
      {"if (true > 1) { true + 1; }", "type mismatch: BOOLEAN > INTEGER"},
      {"return true + 1;", "type mismatch: BOOLEAN + INTEGER"},
      {"if (10 > 1) { if (10 > 1) { return true + false; } return 1; }",
       "unknown operator: BOOLEAN + BOOLEAN"},
      {"foobar", "identifier not found: foobar"},
      {R"raw("Hello" - "World" )raw", "unknown operator: STRING - STRING"},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckErrorObj(obj, test.second);
  }
}

TEST(EvaluatorTest, TestLetStatement) {
  const std::vector<InputExpected<int64_t>> tests = {
      {"let a = 5; a;", 5},
      {"let a = 5 * 5; a;", 25},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckIntObj(obj, test.second);
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
  const std::vector<InputExpected<int64_t>> tests = {
      {"let identity = fn(x) { x; }; identity(5);", 5},
      {"let identity = fn(x) { return x; }; identity(5);", 5},
      {"let double = fn(x) { x * 2; }; double(5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
      {"fn(x) { x; }(5)", 5},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckIntObj(obj, test.second);
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
  CheckIntObj(obj, 4);
}

TEST(EvaluatorTest, TestStringLiteral) {
  const std::string input = R"raw("Hello World!")raw";
  const auto obj = ParseAndEval(input);
  SCOPED_TRACE(input);
  CheckStrObj(obj, "Hello World!");
}

TEST(EvaluatorTest, TestStringConcat) {
  const std::string input = R"raw("Hello" + " " + "World!")raw";
  SCOPED_TRACE(input);
  const auto obj = ParseAndEval(input);
  CheckStrObj(obj, "Hello World!");
}

TEST(EvaluatorTest, TestBuiltinFunctions) {
  using Value = absl::variant<int64_t, std::string>;
  const std::vector<InputExpected<Value>> tests = {
      {R"r(len(""))r", 0},
      {R"r(len("four"))r", 4},
      {R"r(len("hello world"))r", 11},
      {R"r(len(1))r", "argument to `len` not supported, got INTEGER"},
      {R"r(len("one", "two"))r", "wrong number of arguments. got=2, want=1"}};

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);

    switch (test.second.index()) {
      case 0:
        CheckIntObj(obj, std::get<0>(test.second));
        break;
      case 1:
        CheckErrorObj(obj, std::get<1>(test.second));
        break;
      default:
        ASSERT_TRUE(false) << "Should not reach here";
    }
  }
}

TEST(EvaluatorTest, TestArrayLiterals) {
  const std::string input = "[1, 2 * 2, 3 + 3]";
  const auto obj = ParseAndEval(input);
  SCOPED_TRACE(input);
  ASSERT_EQ(obj.Type(), ObjectType::kArray);
  const auto& array = obj.Cast<Array>();
  ASSERT_EQ(array.size(), 3);
  CheckIntObj(array[0], 1);
  CheckIntObj(array[1], 4);
  CheckIntObj(array[2], 6);
}

TEST(EvaluatorTest, TestArrayIndexExpression) {
  using Value = absl::variant<void*, int64_t>;
  const std::vector<InputExpected<Value>> tests = {
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
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);

    switch (test.second.index()) {
      case 0:
        EXPECT_EQ(obj.Type(), ObjectType::kNull);
        break;
      case 1:
        CheckIntObj(obj, std::get<1>(test.second));
        break;
      default:
        ASSERT_FALSE(true) << "Should not reach here";
    }
  }
}

}  // namespace
}  // namespace monkey
