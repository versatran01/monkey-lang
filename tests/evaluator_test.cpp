#include "monkey/evaluator.h"

#include <gtest/gtest.h>

#include "monkey/parser.h"

namespace monkey {
namespace {

Object ParseAndEval(const std::string& input) {
  Parser parser{input};
  const auto program = parser.ParseProgram();
  return Evaluate(program);
}

void CheckIntObject(const Object& obj, int64_t value) {
  ASSERT_EQ(obj.Type(), ObjectType::kInt);
  const auto* ptr = dynamic_cast<IntObject*>(obj.Ptr());
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->value, value);
}

TEST(EvaluatorTest, TestEvalIntergerExpression) {
  const std::vector<std::pair<std::string, int64_t>> tests = {{"5", 5},
                                                              {"10", 10}};

  for (const auto& test : tests) {
    const auto obj = ParseAndEval(test.first);
    CheckIntObject(obj, test.second);
  }
}

}  // namespace
}  // namespace monkey
