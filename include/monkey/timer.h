#pragma once

#include <absl/container/flat_hash_map.h>  // flat_hash_map
#include <absl/strings/string_view.h>      // string_view
#include <absl/time/clock.h>               // GetCurrentTimeNanos
#include <absl/time/time.h>                // Duration

#include <limits>        // numeric_limits
#include <shared_mutex>  // shared_mutex

#include "monkey/stats.h"  // Stats

/// Specialize numeric_limits for absl::Duration (follows integral type)
/// https://codeyarns.com/tech/2015-07-02-max-min-and-lowest-in-c.html
namespace std {
template <>
struct numeric_limits<absl::Duration> {
  static constexpr bool is_specialized = true;
  static constexpr absl::Duration min() { return absl::Nanoseconds(0); }
  static constexpr absl::Duration lowest() { return absl::Nanoseconds(0); }
  static constexpr absl::Duration max() { return absl::InfiniteDuration(); }
};
}  // namespace std

namespace monkey {

/// A simple Timer class, not thread-safe
/// Could've used boost timer but don't want to include boost
class Timer {
 public:
  /// Timer starts right away
  Timer() { Start(); }

  bool IsRunning() const noexcept { return running_; }
  bool IsStopped() const noexcept { return !running_; }
  int64_t NowNs() const { return absl::GetCurrentTimeNanos(); }

  /// Start timer, repeated calls to Start() will update the start time
  void Start();

  /// Stop timer, repeated calls to Stop( have no effect after the 1st
  void Stop();

  /// Resume timer, keep counting from the latest Stop(), repeated calls to
  /// Resume() have no effect
  void Resume();

  /// Return elapsed time as duration, will not stop timer
  int64_t Elapsed() const;

 private:
  int64_t time_ns_{0};  // either start (running) or elapsed (stopped)
  bool running_{false};
};

/// Time statistics
using TimeStats = Stats<absl::Duration>;
std::string Repr(const TimeStats& stats);
std::ostream& operator<<(std::ostream& os, const TimeStats& stats);

/// This is similar to Ceres Solver's ExecutionSummary class, where we record
/// execution statistics (mainly time). Instead of simply record the time, we
/// store a bunch of other statistics using boost accumulator.
class TimerManager {
 public:
  /// A manual timer where user needs to call stop and commit explicitly
  /// Threads-safe
  class ManualTimer {
   public:
    ManualTimer(std::string name, TimerManager* manager)
        : name_{std::move(name)}, manager_{manager} {
      timer_.Start();
    }
    virtual ~ManualTimer() noexcept = default;

    /// Disable copy, allow move
    ManualTimer(const ManualTimer&) = delete;
    ManualTimer& operator=(const ManualTimer&) = delete;
    ManualTimer(ManualTimer&&) noexcept = default;
    ManualTimer& operator=(ManualTimer&&) = default;

    /// Start the timer
    void Start() { timer_.Start(); }

    /// Stop and record the elapsed time after Start
    void Stop();

    /// Commit changes to manager, potentially expensive since it needs to
    /// acquire a lock
    void Commit();

   private:
    Timer timer_;            // actual timer
    TimeStats stats_;        // local stats
    std::string name_;       // name of timer
    TimerManager* manager_;  // ref to manager
  };

  /// A scoped timer that will call stop on destruction
  class ScopedTimer : public ManualTimer {
   public:
    using ManualTimer::ManualTimer;
    ~ScopedTimer() noexcept { Commit(); }
  };

  explicit TimerManager(std::string name = "timers") : name_{std::move(name)} {}

  const std::string& name() const noexcept { return name_; }
  auto size() const noexcept { return stats_dict_.size(); }
  bool empty() const noexcept { return size() == 0; }

  /// Start a ManualTimer by name, need to manually stop the returned timer.
  /// Elapsed time will automatically added to the stats when stopped.
  /// After stop one can just call timer.Start() to restart.
  /// Need to call Commit() to aggregate stats
  ManualTimer Manual(std::string name) { return {std::move(name), this}; }

  /// Returns a ScopedTimer (already started) and will stop when out of scope
  ScopedTimer Scoped(std::string name) { return {std::move(name), this}; }

  /// Thread-safe update, aggregate stats
  void Update(absl::string_view timer_name, const TimeStats& stats);

  /// Returns a copy of the stats under timer_name
  /// If not found returns empty stats
  TimeStats GetStats(absl::string_view timer_name) const;

  /// Return a string of timer statistics
  std::string ReportAll() const;
  std::string Report(absl::string_view timer_name) const;

 private:
  using StatsDict = absl::flat_hash_map<std::string, TimeStats>;

  std::string name_;                 // name of the manager
  StatsDict stats_dict_;             // all stats
  mutable std::shared_mutex mutex_;  // reader-writer mutex
};

}  // namespace monkey
