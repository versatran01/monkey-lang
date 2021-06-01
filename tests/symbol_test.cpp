#include "monkey/symbol.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

using namespace monkey;
using namespace std::string_literals;
using ::testing::ContainerEq;

TEST(SymbolTest, TestSymbolDefine) {
  const SymbolDict tests = {
      {"a", {"a", SymbolScope::kGlobal, 0}},
      {"b", {"b", SymbolScope::kGlobal, 1}},
      {"c", {"c", SymbolScope::kLocal, 0}},
      {"d", {"d", SymbolScope::kLocal, 1}},
      {"e", {"e", SymbolScope::kLocal, 0}},
      {"f", {"f", SymbolScope::kLocal, 1}},
  };

  SymbolTable global;
  EXPECT_TRUE(global.IsGlobal());
  auto& a = global.Define("a");
  EXPECT_EQ(a, tests.at("a"));
  EXPECT_EQ(global.NumDefs(), 1);

  auto& b = global.Define("b");
  EXPECT_EQ(b, tests.at("b"));
  EXPECT_EQ(global.NumDefs(), 2);

  auto local1 = SymbolTable(&global);
  EXPECT_FALSE(local1.IsGlobal());
  auto& c = local1.Define("c");
  EXPECT_EQ(c, tests.at("c"));

  auto& d = local1.Define("d");
  EXPECT_EQ(d, tests.at("d"));

  auto local2 = SymbolTable(&local1);
  EXPECT_FALSE(local2.IsGlobal());
  auto& e = local2.Define("e");
  EXPECT_EQ(e, tests.at("e"));

  auto& f = local2.Define("f");
  EXPECT_EQ(f, tests.at("f"));
}

TEST(SymbolTest, TestSymbolResolve) {
  const std::vector<Symbol> tests = {
      {"a", SymbolScope::kGlobal, 0},
      {"b", SymbolScope::kGlobal, 1},
  };

  SymbolTable global;
  global.Define("a");
  global.Define("b");
  EXPECT_EQ(global.NumDefs(), 2);

  auto a = global.Resolve("a");
  ASSERT_TRUE(a.has_value());
  EXPECT_EQ(*a, tests[0]);

  auto b = global.Resolve("b");
  ASSERT_TRUE(b.has_value());
  EXPECT_EQ(*b, tests[1]);

  auto c = global.Resolve("c");
  EXPECT_FALSE(c.has_value());
}

TEST(SymbolTest, TestResolveLocal) {
  SymbolTable global;
  global.Define("a");
  global.Define("b");

  auto local = SymbolTable(&global);
  local.Define("c");
  local.Define("d");

  const std::vector<Symbol> symbols = {
      {"a", SymbolScope::kGlobal, 0},
      {"b", SymbolScope::kGlobal, 1},
      {"c", SymbolScope::kLocal, 0},
      {"d", SymbolScope::kLocal, 1},
  };

  for (const auto& sym : symbols) {
    auto res = local.Resolve(sym.name);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, sym);
  }
}

TEST(SymbolTest, TestResolveNestedLocal) {
  SymbolTable global;
  global.Define("a");
  global.Define("b");

  auto local1 = SymbolTable(&global);
  local1.Define("c");
  local1.Define("d");

  auto local2 = SymbolTable(&local1);
  local2.Define("e");
  local2.Define("f");

  const std::vector<Symbol> symbols1 = {
      {"a", SymbolScope::kGlobal, 0},
      {"b", SymbolScope::kGlobal, 1},
      {"c", SymbolScope::kLocal, 0},
      {"d", SymbolScope::kLocal, 1},
  };

  for (const auto& sym : symbols1) {
    auto res = local1.Resolve(sym.name);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, sym);
  }

  const std::vector<Symbol> symbols2 = {
      {"a", SymbolScope::kGlobal, 0},
      {"b", SymbolScope::kGlobal, 1},
      {"e", SymbolScope::kLocal, 0},
      {"f", SymbolScope::kLocal, 1},
  };

  for (const auto& sym : symbols2) {
    auto res = local2.Resolve(sym.name);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, sym);
  }
}

TEST(SymbolTest, TestDefineResolveBuiltin) {
  auto global = SymbolTable{};
  auto local1 = SymbolTable(&global);
  auto local2 = SymbolTable(&local1);

  const std::vector<Symbol> symbols = {
      {"a", SymbolScope::kBuiltin, 0},
      {"c", SymbolScope::kBuiltin, 1},
      {"e", SymbolScope::kBuiltin, 2},
      {"f", SymbolScope::kBuiltin, 3},
  };

  for (const auto& sym : symbols) {
    global.DefineBuiltin(sym.name, sym.index);
  }

  std::vector<SymbolTable*> tables{&global, &local1, &local2};
  for (const auto& table : tables) {
    for (const auto& sym : symbols) {
      auto res = table->Resolve(sym.name);
      ASSERT_TRUE(res.has_value());
      EXPECT_EQ(*res, sym);
    }
  }
}

struct SymbolTest {
  SymbolTable table;
  std::vector<Symbol> symbols;
  std::vector<Symbol> free;
};

TEST(SymbolTest, TestResolveFree) {
  auto global = SymbolTable{};
  global.Define("a");
  global.Define("b");

  auto local1 = SymbolTable{&global};
  local1.Define("c");
  local1.Define("d");

  auto local2 = SymbolTable{&local1};
  local2.Define("e");
  local2.Define("f");

  std::vector<SymbolTest> tests = {
      {
          local1,
          {
              {"a", SymbolScope::kGlobal, 0},
              {"b", SymbolScope::kGlobal, 1},
              {"c", SymbolScope::kLocal, 0},
              {"d", SymbolScope::kLocal, 1},
          },
      },
      {
          local2,
          {
              {"a", SymbolScope::kGlobal, 0},
              {"b", SymbolScope::kGlobal, 1},
              {"c", SymbolScope::kFree, 0},
              {"d", SymbolScope::kFree, 1},
              {"e", SymbolScope::kLocal, 0},
              {"f", SymbolScope::kLocal, 1},
          },
          {
              {"c", SymbolScope::kLocal, 0},
              {"d", SymbolScope::kLocal, 1},
          },
      },
  };

  for (auto& test : tests) {
    for (const auto& sym : test.symbols) {
      auto res = test.table.Resolve(sym.name);
      SCOPED_TRACE(test.table.Repr());
      ASSERT_TRUE(res.has_value());
      ASSERT_EQ(*res, sym);
    }

    // Check free symbols
    EXPECT_THAT(test.table.FreeSymbols(), ContainerEq(test.free));
  }
}

TEST(SymbolTest, TestResolveUnResolvableFree) {
  auto global = SymbolTable{};
  global.Define("a");

  auto local1 = SymbolTable{&global};
  local1.Define("c");

  auto local2 = SymbolTable{&local1};
  local2.Define("e");
  local2.Define("f");

  const std::vector<Symbol> symbols = {
      {"a", SymbolScope::kGlobal, 0},
      {"c", SymbolScope::kFree, 0},
      {"e", SymbolScope::kLocal, 0},
      {"f", SymbolScope::kLocal, 1},
  };

  for (const auto& sym : symbols) {
    const auto res = local2.Resolve(sym.name);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, sym);
  }

  const std::vector<std::string> names = {"b"s, "d"s};
  for (const auto& name : names) {
    const auto res = local2.Resolve(name);
    EXPECT_FALSE(res.has_value());
  }
}

}  // namespace
