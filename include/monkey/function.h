#pragma once

#include "monkey/ast.h"
#include "monkey/object.h"

namespace monkey {

class Environment;

struct FunctionObject final : public ObjectBase {
  FunctionObject() : ObjectBase{ObjectType::kFunction} {}
  std::string InspectImpl() const override;
  auto NumParams() const noexcept { return params.size(); }

  std::vector<Identifier> params;
  BlockStatement body;
  Environment* env{nullptr};
};

}  // namespace monkey
