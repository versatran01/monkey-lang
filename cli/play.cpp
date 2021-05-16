#include <fmt/ranges.h>
#include <glog/logging.h>

int main() {
  LOG(INFO) << "INFO";
  DLOG(INFO) << "DINFO";
  CHECK(false);
}
