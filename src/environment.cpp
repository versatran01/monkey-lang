#include "monkey/environment.h"

#include <absl/strings/str_join.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include <iostream>

namespace monkey {

Object Environment::Get(const std::string& name) const {
  const auto it = store_.find(name);
  // Found
  if (it != store_.end()) {
    return it->second;
  }

  // Not found, try outer env
  if (outer_ != nullptr) {
    return outer_->Get(name);
  }

  return Object{};
}

Object& Environment::Set(const std::string& name, const Object& obj) {
  return store_[name] = obj;
}

Environment MakeEnclosedEnv(Environment* env) { return Environment{env}; }

std::ostream& operator<<(std::ostream& os, const Environment& env) {
  auto pf = absl::PairFormatter(
      absl::AlphaNumFormatter(), ": ", [](std::string* out, const Object& obj) {
        out->append(obj.Inspect());
      });
  os << fmt::format("[{}]", absl::StrJoin(env.store_, " | ", pf));
  if (env.outer_) {
    os << "->" << *env.outer_;
  }
  return os;
}

}  // namespace monkey
