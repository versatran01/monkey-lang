#pragma once

#include "monkey/ast.h"
#include "monkey/environment.h"
#include "monkey/object.h"

namespace monkey {

struct FunctionObject final : public ObjectBase {
  FunctionObject(const std::vector<Identifier>& params = {},
                 const BlockStatement& body = {},
                 Environment* env = nullptr)
      : ObjectBase{ObjectType::kFunction},
        params{params},
        body{body},
        env{env} {}
  std::string InspectImpl() const override;
  auto NumParams() const noexcept { return params.size(); }

  std::vector<Identifier> params;
  BlockStatement body;
  Environment* env{nullptr};
};

}  // namespace monkey
