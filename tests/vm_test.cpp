#include "monkey/vm.h"

#include <gtest/gtest.h>

#include "monkey/compiler.h"
#include "monkey/object.h"
#include "monkey/parser.h"

namespace {
using namespace monkey;

struct VmTest {
  std::string input;
  Object obj;
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
  vm.Run(*bc);

  const auto& top = vm.Top();
  EXPECT_EQ(top, test.obj);
}

TEST(VmTest, TestIntArithmetic) {
  const std::vector<VmTest> tests = {
      {"1", IntObj(1)},
      {"2", IntObj(2)},
      {"1 + 2", IntObj(3)},
  };

  for (const auto& test : tests) {
    SCOPED_TRACE(test.input);
    CheckVm(test);
  }
}

}  // namespace
