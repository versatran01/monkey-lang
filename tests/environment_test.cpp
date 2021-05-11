#include "monkey/environment.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(EnvironmentTest, TestGetSet) {
  Environment env;
  env.Set("i", IntObject(1));
  EXPECT_EQ(env.size(), 1);
  EXPECT_EQ(env.Get("_"), nullptr);

  const auto* o1p = env.Get("i");
  ASSERT_NE(o1p, nullptr);
  EXPECT_EQ(o1p->Inspect(), "1");
  EXPECT_EQ(o1p->Cast<int64_t>(), 1);
  //  EXPECT_EQ(*o1p->PtrCast<int64_t>(), 1);
}

TEST(EnvironmentTest, TestOstream) {
  Environment env;
  env.Set("i", IntObject(1));
  env.Set("b", BoolObject(true));
  LOG(INFO) << env;
}

}  // namespace
}  // namespace monkey
