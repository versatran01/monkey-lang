#include "monkey/evaluator.h"

#include <absl/types/variant.h>
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
  return eval.Evaluate(program);
}

void CheckBoolObject(const Object& obj, bool value) {
  ASSERT_EQ(obj.Type(), ObjectType::kBool);
  EXPECT_EQ(obj.PtrCast<BoolObject>()->value, value);
}

void CheckIntObject(const Object& obj, int64_t value) {
  ASSERT_EQ(obj.Type(), ObjectType::kInt);
  EXPECT_EQ(obj.PtrCast<IntObject>()->value, value);
}

void CheckErrorObj(const Object& obj, const std::string& msg) {
  ASSERT_EQ(obj.Type(), ObjectType::kError);
  EXPECT_EQ(obj.PtrCast<ErrorObject>()->msg, msg);
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
    const auto obj = ParseAndEval(test.first);
    ASSERT_EQ(obj.Type(), ObjectType::kInt);
    EXPECT_EQ(obj.PtrCast<IntObject>()->value, test.second);
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
    CheckBoolObject(obj, test.second);
  }
}

TEST(EvaluatorTest, TestBangOperator) {
  const std::vector<InputExpected<bool>> tests = {
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.first);
    const auto obj = ParseAndEval(test.first);
    CheckBoolObject(obj, test.second);
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
        CheckIntObject(obj, std::get<1>(test.second));
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
    CheckIntObject(obj, test.second);
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
    CheckIntObject(obj, test.second);
  }
}

}  // namespace
}  // namespace monkey
