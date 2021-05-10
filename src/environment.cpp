#include "monkey/environment.h"

namespace monkey {

Object* Environment::Get(absl::string_view name) {
  const auto it = store_.find(name);
  if (it == store_.end()) {
    // Not found
    if (outer_ == nullptr) {
      return nullptr;
    } else {
      return outer_->Get(name);
    }
  } else {
    // Found
    return &it->second;
  }
}

void Environment::Set(const std::string& name, const Object& obj) {
  auto it = store_.find(name);
  if (it == store_.end()) {
    store_.insert({name, obj});
  } else {
    it->second = obj;
  }
}

Environment ExtendEnv(Environment& env) { return Environment{&env}; }

}  // namespace monkey
