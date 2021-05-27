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
}  // namespace
