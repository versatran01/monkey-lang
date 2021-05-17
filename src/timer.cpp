#include "monkey/timer.h"

#include <absl/strings/str_join.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

namespace {

struct NameFmt {
  void operator()(std::string* out, absl::string_view name) const {
    out->append(fmt::format("[{:<16}]", name));
  }
};

struct StatsFmt {
  void operator()(std::string* out, const TimeStats& stats) const {
    out->append(ToString(stats));
  }
};

std::string ReportFormat(absl::string_view timer_name, const TimeStats& stats) {
  std::string res;
  NameFmt{}(&res, timer_name);
  StatsFmt{}(&res, stats);
  return res;
}

}  // namespace

std::string ToString(const TimeStats& stats) {
  return fmt::format(
      " n: {:<8} | sum: {:<16} | min: {:<16} | max: {:<16} | "
      "mean: {:<16} | last: {:<16} |",
      stats.count(), stats.sum(), stats.min(), stats.max(), stats.mean(),
      stats.last());
}

std::ostream& operator<<(std::ostream& os, const TimeStats& stats) {
  return os << ToString(stats);
}

void Timer::Start() {
  running_ = true;
  time_ns_ = NowNs();  // time_ns_ is start time
}

inline void Timer::Stop() {
  if (!running_) {
    return;
  }
  running_ = false;
  time_ns_ = NowNs() - time_ns_;  // time_ns_ is elpased time
}

void Timer::Resume() {
  if (running_) {
    // noop if still running
    return;
  }
  const auto prev_elapsed = time_ns_;
  Start();                   // time_ns is now the new start time
  time_ns_ -= prev_elapsed;  // subtract prev_elapsed from new start time
}

int64_t Timer::Elapsed() const {
  if (!running_) return time_ns_;  // time_ns_ is already elapsed
  return NowNs() - time_ns_;       // time_ns_ is when the timer started
}

void TimerManager::ManualTimer::Stop() {
  CHECK(timer_.IsRunning()) << "Calling Stop() but timer is not running";
  timer_.Stop();
  stats_.Add(absl::Nanoseconds(timer_.Elapsed()));
  //  stats_.Add(static_cast<double>(timer_.Elapsed()));
}

void TimerManager::ManualTimer::Commit() {
  if (timer_.IsRunning()) {
    Stop();
  }
  manager_->Update(name_, stats_);
  stats_ = TimeStats{};  // reset stats
}

void TimerManager::Update(absl::string_view timer_name,
                          const TimeStats& stats) {
  std::unique_lock lock(mutex_);
  stats_dict_[timer_name] += stats;
}

auto TimerManager::GetStats(absl::string_view timer_name) const -> TimeStats {
  std::shared_lock lock(mutex_);
  const auto it = stats_dict_.find(timer_name);
  if (it != stats_dict_.end()) {
    return it->second;
  }
  LOG(WARNING) << fmt::format("Timer [{}] not in TimerManager [{}].",
                              timer_name, name_);
  return {};
}

std::string TimerManager::Report(absl::string_view timer_name) const {
  return ReportFormat(timer_name, GetStats(timer_name));
}

std::string TimerManager::ReportAll() const {
  return fmt::format(
      "Timer Summary: {}\n{}", name_,
      absl::StrJoin(stats_dict_, "\n",
                    absl::PairFormatter(NameFmt{}, "", StatsFmt{})));
}

}  // namespace monkey
