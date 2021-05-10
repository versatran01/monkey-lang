#include "monkey/environment.h"

#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(EnvironmentTest, TestGetSet) {
  Environment env;
  IntObject o1{1};
  env.Set("a", o1);
  EXPECT_EQ(env.size(), 1);
  EXPECT_EQ(env.Get("_"), nullptr);
  const auto o1p = env.Get("a");
  ASSERT_NE(o1p, nullptr);
  EXPECT_EQ(o1p->PtrCast<IntObject>()->value, o1.value);

  env.Set("a", BoolObject{true});
  EXPECT_EQ(env.size(), 1);
  const auto o2p = env.Get("a");
  ASSERT_NE(o2p, nullptr);
  EXPECT_EQ(o2p->PtrCast<BoolObject>()->value, true);
}

}  // namespace
}  // namespace monkey
