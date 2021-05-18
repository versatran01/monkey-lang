#include "monkey/object.h"

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>

namespace {
using namespace monkey;

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
  EXPECT_EQ(IntObj(1).Cast<IntType>(), 1);
  EXPECT_EQ(ErrorObj("error").Cast<std::string>(), "error");
}

TEST(ObjectTest, TestArray) {
  const auto array = ArrayObj({IntObj(1), IntObj(2)});
  EXPECT_EQ(array.Inspect(), "[1, 2]");
}

TEST(ObjectTest, TestStringHashKey) {
  const auto hello1 = StrObj("Hello World");
  const auto hello2 = StrObj("Hello World");
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(
      {StrObj("1"), IntObj(1), BoolObj(true)}));
}

TEST(ObjectTest, TestSameType) {
  EXPECT_TRUE(ObjOfSameType(ObjectType::kInt, IntObj(1)));
  EXPECT_TRUE(ObjOfSameType(ObjectType::kInt, IntObj(1), IntObj(2)));
  EXPECT_FALSE(ObjOfSameType(ObjectType::kInt, BoolObj(true)));
  EXPECT_FALSE(ObjOfSameType(ObjectType::kInt, IntObj(1), BoolObj(true)));
}

}  // namespace
