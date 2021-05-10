#include "monkey/environment.h"

namespace monkey {

const ObjectBase *Environment::Get(absl::string_view name) const {
  const auto it = store_.find(name);
  if (it != store_.end()) {
    return it->second;
  }
  return nullptr;
}

void Environment::Set(const std::string &name, const Object &obj) {
  store_[name] = obj.Ptr();
}

}  // namespace monkey
