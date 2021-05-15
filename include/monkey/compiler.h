#pragma once

#include "monkey/ast.h"
#include "monkey/code.h"
#include "monkey/object.h"

namespace monkey {

struct Bytecode {
  Instruction inst;
  std::vector<Object> consts;
};

class Compiler {
 public:
  Bytecode Compile(const AstNode& node);
};

}  // namespace monkey
