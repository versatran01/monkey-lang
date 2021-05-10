#include "monkey/object.h"

#include <fmt/ostream.h>
#include <gtest/gtest.h>

namespace monkey {
namespace {

TEST(ObjecTest, TestOstream) {
  std::string s;
  {
    s = fmt::format("{}", ObjectBase{}.Type());
    EXPECT_EQ(s, "INVALID");
  }

  {
    s = fmt::format("{}", IntObject{}.Type());
    EXPECT_EQ(s, "INTEGER");
  }

  {
    s = fmt::format("{}", BoolObject{}.Type());
    EXPECT_EQ(s, "BOOLEAN");
  }
}

}  // namespace
}  // namespace monkey
