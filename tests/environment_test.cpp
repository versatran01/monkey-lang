

#include "monkey/environment.h"

#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(EnvironmentTest, TestGetSet) {
  Environment env;
  IntObject o1{1};
  env.Set("a", o1);
  EXPECT_EQ(env.Get("_"), nullptr);
  auto o1p = env.Get("a");
  EXPECT_EQ(o1p->PtrCast<IntObject>()->value, o1.value);
}

}  // namespace
}  // namespace monkey
