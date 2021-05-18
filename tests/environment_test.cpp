#include "monkey/environment.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace {

using namespace monkey;

TEST(EnvironmentTest, TestGetSet) {
  Environment env;
  env.Set("i", IntObj(1));
  EXPECT_EQ(env.size(), 1);
  EXPECT_FALSE(env.Get("_").Ok());

  const auto o1 = env.Get("i");
  ASSERT_TRUE(o1.Ok());
  EXPECT_EQ(o1.Inspect(), "1");
  EXPECT_EQ(o1.Cast<IntType>(), 1);
}

TEST(EnvironmentTest, TestOstream) {
  Environment env;
  env.Set("i", IntObj(1));
  env.Set("b", BoolObj(true));
  LOG(INFO) << env;
}

}  // namespace
