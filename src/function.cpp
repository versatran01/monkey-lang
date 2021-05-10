#include "monkey/function.h"

#include <absl/strings/str_join.h>
#include <fmt/core.h>

namespace monkey {

std::string FunctionObject::InspectImpl() const {
  return fmt::format(
      "fn({}) {\n{}\n}",
      absl::StrJoin(params,
                    ", ",
                    [](std::string* out, const Identifier& ident) {
                      out->append(ident.value);
                    }),
      body.String());
}

}  // namespace monkey
