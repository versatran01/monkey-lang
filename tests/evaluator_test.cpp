#include "monkey/evaluator.h"

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

}  // namespace
}  // namespace monkey
