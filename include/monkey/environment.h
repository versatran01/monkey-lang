#pragma once

#include <absl/container/flat_hash_map.h>

#include <iosfwd>

#include "monkey/object.h"

namespace monkey {

class Environment {
 public:
  explicit Environment(Environment* outer = nullptr) : outer_{outer} {}

  Object Get(const std::string& name) const;
  Object& Set(const std::string& name, const Object& obj);

  auto size() const noexcept { return store_.size(); }
  auto empty() const noexcept { return store_.empty(); }

  friend std::ostream& operator<<(std::ostream& os, const Environment& env);

 private:
  absl::flat_hash_map<std::string, Object> store_;
  Environment* outer_{nullptr};
};

Environment MakeEnclosedEnv(Environment* env);

}  // namespace monkey
