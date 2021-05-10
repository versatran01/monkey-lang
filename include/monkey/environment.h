#pragma once

#include <absl/container/node_hash_map.h>
#include <absl/strings/string_view.h>

#include "monkey/object.h"

namespace monkey {

class Environment {
 public:
  const ObjectBase* Get(absl::string_view name) const;
  void Set(const std::string& name, const Object& obj);

 private:
  absl::node_hash_map<std::string, const ObjectBase*> store_;
  Environment* outer_{nullptr};
};

}  // namespace monkey
