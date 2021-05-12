#include "monkey/object.h"

#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(ObjecTest, TestInspect) {
  EXPECT_EQ(NullObj().Inspect(), "Null");
  EXPECT_EQ(BoolObj(true).Inspect(), "true");
  EXPECT_EQ(BoolObj(false).Inspect(), "false");
  EXPECT_EQ(IntObj(1).Inspect(), "1");
  EXPECT_EQ(ErrorObj("error").Inspect(), "error");
}

TEST(ObjecTest, TestCast) {
  EXPECT_THROW(NullObj().Cast<bool>(), absl::bad_any_cast);
  EXPECT_THROW(IntObj(1).Cast<bool>(), absl::bad_any_cast);
  EXPECT_THROW(IntObj(2).Cast<std::string>(), absl::bad_any_cast);

  EXPECT_EQ(BoolObj(true).Cast<bool>(), true);
  EXPECT_EQ(IntObj(1).Cast<int64_t>(), 1);
  EXPECT_EQ(ErrorObj("error").Cast<std::string>(), "error");
}

}  // namespace
}  // namespace monkey
