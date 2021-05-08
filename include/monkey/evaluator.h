#pragma once

#include "monkey/ast.h"
#include "monkey/object.h"

namespace monkey {

class Evaluator {
 public:
  Object Evaluate(const Program& node);
  Object Evaluate(const Statement& stmt);
  Object Evaluate(const Expression& expr);

  Object EvalStatements(const std::vector<Statement>& stmts);

 private:
  static BoolObject kTrueObject;
  static BoolObject kFalseObject;
  static NullObject kNullObject;
};

}  // namespace monkey
