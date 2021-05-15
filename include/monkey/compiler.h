#pragma once

#include "monkey/code.h"
#include "monkey/object.h"

namespace monkey {

struct Bytecode {
  ByteVec intstructions;
  std::vector<Object> constatns;
};

class Compiler {
 public:
  Bytecode Compile(const AstNode& node);

 private:
  ByteVec intstructions;
  std::vector<Object> constatns;
};

}  // namespace monkey
