#include "monkey/environment.h"

namespace monkey {

const Object *Environment::Get2(absl::string_view name) const {
  const auto it = store_.find(name);
  if (it != store_.end()) {
    return &it->second;
  }
  return nullptr;
}

void Environment::Set2(const std::string &name, const Object &obj) {
  store_.insert({name, obj});
}

}  // namespace monkey
