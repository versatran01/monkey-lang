#pragma once

#include "monkey/object.h"

namespace monkey {

struct Frame {
  const Instruction& Ins() const noexcept { return func.ins; }

  CompiledFunc func;
  size_t ip;
};

}  // namespace monkey
