#pragma once

#include "monkey/ast.h"
#include "monkey/code.h"
#include "monkey/object.h"

namespace monkey {

struct Bytecode {
  Instruction intstruction;
  std::vector<Object> constants;
};

class Compiler {
 public:
  Bytecode Compile(const AstNode& node);
};

}  // namespace monkey
