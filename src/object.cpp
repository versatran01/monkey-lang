#include "monkey/object.h"

#include <absl/container/flat_hash_map.h>

namespace monkey {

namespace {

const auto gObjectTypeStrings = absl::flat_hash_map<ObjectType, std::string>{
    {ObjectType::kInvalid, "INVALID"},
    {ObjectType::kNull, "NULL"},
    {ObjectType::kInt, "INTEGER"},
    {ObjectType::kBool, "BOOLEAN"},
    {ObjectType::kReturn, "RETURN"},
    {ObjectType::kError, "ERROR"},
    {ObjectType::kFunction, "FUNCTION"},
    {ObjectType::kIndirect, "INDIRECT"},
};

}  // namespace

std::ostream& operator<<(std::ostream& os, ObjectType type) {
  return os << gObjectTypeStrings.at(type);
}

}  // namespace monkey
