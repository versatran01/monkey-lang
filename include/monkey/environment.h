#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/strings/string_view.h>

#include <iosfwd>

#include "monkey/object.h"

namespace monkey {

class Environment {
 public:
  explicit Environment(const Environment* outer = nullptr) : outer_{outer} {}

  Object Get(absl::string_view name) const;
  Object& Set(absl::string_view name, const Object& obj);

  auto size() const noexcept { return store_.size(); }
  auto empty() const noexcept { return store_.empty(); }

  friend std::ostream& operator<<(std::ostream& os, const Environment& env);

 private:
  absl::flat_hash_map<std::string, Object> store_;
  const Environment* outer_{nullptr};
};

}  // namespace monkey
