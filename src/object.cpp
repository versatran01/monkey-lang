#include "monkey/object.h"

#include <absl/container/flat_hash_map.h>

namespace monkey {

namespace {

const auto gObjectTypeStrings = absl::flat_hash_map<ObjectType, std::string>{
    {ObjectType::kInvalid, "InvalidObj"},
    {ObjectType::kNull, "NullObj"},
    {ObjectType::kInt, "IntObj"},
    {ObjectType::kBool, "BoolObj"},
    {ObjectType::kReturn, "ReturnObj"}};

}  // namespace

std::ostream &operator<<(std::ostream &os, ObjectType type) {
  return os << gObjectTypeStrings.at(type);
}

}  // namespace monkey
