#include "monkey/object.h"

#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(ObjecTest, TestInspect) {
  EXPECT_EQ(NullObject().Inspect(), "Null");
  EXPECT_EQ(BoolObject(true).Inspect(), "true");
  EXPECT_EQ(BoolObject(false).Inspect(), "false");
  EXPECT_EQ(IntObject(1).Inspect(), "1");
  EXPECT_EQ(ErrorObject("error").Inspect(), "error");
}

TEST(ObjecTest, TestCast) {
  EXPECT_THROW(NullObject().Cast<bool>(), absl::bad_any_cast);
  EXPECT_THROW(IntObject(1).Cast<bool>(), absl::bad_any_cast);
  EXPECT_THROW(IntObject(2).Cast<std::string>(), absl::bad_any_cast);

  EXPECT_EQ(BoolObject(true).Cast<bool>(), true);
  EXPECT_EQ(IntObject(1).Cast<int64_t>(), 1);
  EXPECT_EQ(ErrorObject("error").Cast<std::string>(), "error");
}

}  // namespace
}  // namespace monkey
