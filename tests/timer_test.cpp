#include "monkey/timer.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace {

using namespace monkey;

TEST(TimerTest, TestDefaultCtor) {
  auto tm = TimerManager{"test"};

  EXPECT_EQ(tm.name(), "test");
  EXPECT_EQ(tm.size(), 0);
  EXPECT_TRUE(tm.empty());
}

TEST(TimerTest, TestGetStats) {
  auto tm = TimerManager{"test"};
  auto st = tm.GetStats("a");
  EXPECT_FALSE(st.ok());
  auto mt = tm.Manual("a");
  mt.Start();
  mt.Stop();
  mt.Commit();
  st = tm.GetStats("a");
  EXPECT_TRUE(st.ok());
}

TEST(TimerTest, TestManualTimer) {
  const int n = 10;
  const int t = 10;
  auto manager = TimerManager{"test"};
  auto mt = manager.Manual("m");

  for (int i = 0; i < n; ++i) {
    mt.Start();
    absl::SleepFor(absl::Milliseconds(t));
    mt.Stop();
  }
  mt.Commit();

  EXPECT_EQ(manager.name(), "test");
  EXPECT_EQ(manager.size(), 1);
  EXPECT_FALSE(manager.empty());

  const auto s = manager.GetStats("m");
  EXPECT_EQ(s.count(), n);
  EXPECT_GE(s.min(), absl::Milliseconds(t - 1));
  EXPECT_LE(s.min(), absl::Milliseconds(t + 1));
  EXPECT_GE(s.max(), absl::Milliseconds(t - 1));
  EXPECT_LE(s.max(), absl::Milliseconds(t + 1));
  EXPECT_GE(s.mean(), absl::Milliseconds(t - 1));
  EXPECT_LE(s.mean(), absl::Milliseconds(t + 1));
}

TEST(TimerTest, TestScopedTimer) {
  auto manager = TimerManager{"test"};

  const int n = 10;
  const int t = 10;

  for (int i = 0; i < n; ++i) {
    auto st = manager.Scoped("s");
    absl::SleepFor(absl::Milliseconds(t));
  }

  EXPECT_EQ(manager.name(), "test");
  EXPECT_EQ(manager.size(), 1);
  EXPECT_FALSE(manager.empty());

  const auto s = manager.GetStats("s");
  EXPECT_EQ(s.count(), n);
  EXPECT_GE(s.min(), absl::Milliseconds(t - 1));
  EXPECT_LE(s.min(), absl::Milliseconds(t + 1));
  EXPECT_GE(s.max(), absl::Milliseconds(t - 1));
  EXPECT_LE(s.max(), absl::Milliseconds(t + 1));
  EXPECT_GE(s.mean(), absl::Milliseconds(t - 1));
  EXPECT_LE(s.mean(), absl::Milliseconds(t + 1));
}

TEST(TimerTest, TestReport) {
  const int n = 10;
  const int t = 1;
  auto tm = TimerManager{"test"};

  for (int i = 0; i < n; ++i) {
    auto st = tm.Scoped("s1");
    absl::SleepFor(absl::Milliseconds(t));
  }

  for (int i = 0; i < n; ++i) {
    auto st = tm.Scoped("s2");
    absl::SleepFor(absl::Milliseconds(t));
  }

  LOG(INFO) << tm.Report("s1");
  LOG(INFO) << tm.Report("s3");  // should be empty
  LOG(INFO) << tm.ReportAll();
}

}  // namespace
