#pragma once

#include <absl/container/node_hash_map.h>
#include <absl/strings/string_view.h>

#include "monkey/object.h"

namespace monkey {

class Environment {
 public:
  explicit Environment(Environment* outer = nullptr) : outer_{outer} {}

  Object* Get(absl::string_view name);
  void Set(const std::string& name, const Object& obj);

  auto size() const noexcept { return store_.size(); }
  auto empty() const noexcept { return store_.empty(); }

 private:
  absl::node_hash_map<std::string, Object> store_;
  Environment* outer_{nullptr};  // TODO: just ptr?
};

Environment ExtendEnv(Environment& env);

}  // namespace monkey
