#pragma once

#include <absl/container/node_hash_map.h>
#include <absl/strings/string_view.h>

#include <iosfwd>

#include "monkey/object.h"

namespace monkey {

class Environment {
 public:
  explicit Environment(Environment* outer = nullptr) : outer_{outer} {}

  Object const* Get(absl::string_view name) const;
  Object& Set(const std::string& name, const Object& obj);

  auto size() const noexcept { return store_.size(); }
  auto empty() const noexcept { return store_.empty(); }

  friend std::ostream& operator<<(std::ostream& os, const Environment& env);

 private:
  absl::node_hash_map<std::string, Object> store_;
  Environment* outer_{nullptr};
};

Environment MakeEnclosedEnv(Environment& env);

}  // namespace monkey
