#include "monkey/evaluator.h"

#include <absl/types/variant.h>
#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace monkey {
namespace {

Object ParseAndEval(const std::string& input) {
  Parser parser{input};
  const auto program = parser.ParseProgram();
  Evaluator eval;
  return eval.Evaluate(program);
}

void CheckNullObject(const Object& obj) {}

void CheckIntObject(const Object& obj, int64_t value) {
  ASSERT_EQ(obj.Type(), ObjectType::kInt);
  const auto* ptr = static_cast<IntObject*>(obj.Ptr());
  EXPECT_EQ(ptr->value, value);
}

void CheckBoolObject(const Object& obj, bool value) {
  ASSERT_EQ(obj.Type(), ObjectType::kBool);
  const auto* ptr = static_cast<BoolObject*>(obj.Ptr());
  EXPECT_EQ(ptr->value, value);
}

TEST(EvaluatorTest, TestEvalIntergerExpression) {
  const std::vector<std::pair<std::string, int64_t>> tests = {
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
    CheckIntObject(obj, test.second);
  }
}

TEST(EvaluatorTest, TestEvalBooleanExpression) {
  const std::vector<std::pair<std::string, bool>> tests = {
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
    const auto obj = ParseAndEval(test.first);
    CheckBoolObject(obj, test.second);
  }
}

TEST(EvaluatorTest, TestBangOperator) {
  const std::vector<std::pair<std::string, bool>> tests = {
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true},
  };

  for (const auto& test : tests) {
    const auto obj = ParseAndEval(test.first);
    CheckBoolObject(obj, test.second);
  }
}

TEST(EvaluatorTest, TestIfElseExpression) {
  using Value = absl::variant<void*, int64_t>;
  const std::vector<std::pair<std::string, Value>> tests = {
      {"if (true) { 10 }", 10},
      {"if (false) { 10 }", nullptr},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 > 2) { 10 }", nullptr},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if (1 < 2) { 10 } else { 20 }", 10},
  };

  for (const auto& test : tests) {
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

}  // namespace
}  // namespace monkey
