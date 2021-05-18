#include "monkey/vm.h"

#include <absl/types/variant.h>
#include <gtest/gtest.h>

#include "monkey/compiler.h"
#include "monkey/object.h"
#include "monkey/parser.h"

namespace {
using namespace monkey;

using Value = absl::variant<int, bool>;

struct VmTest {
  std::string input;
  Value value;
};

Program Parse(const std::string& input) {
  Parser parser{input};
  return parser.ParseProgram();
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
      EXPECT_EQ(vm.last(), IntObj(std::get<0>(test.value)));
      break;
    case 1:
      EXPECT_EQ(vm.last(), BoolObj(std::get<1>(test.value)));
      break;
    default:
      ASSERT_FALSE(true) << "Unhandeld type";
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
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

}  // namespace
