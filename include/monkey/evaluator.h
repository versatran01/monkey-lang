#pragma once

#include "monkey/ast.h"
#include "monkey/object.h"

namespace monkey {

class Evaluator {
 public:
  Object Evaluate(const Program& node) const;
  Object Evaluate(const Statement& stmt) const;
  Object Evaluate(const Expression& expr) const;

 private:
  Object EvalStatements(const std::vector<Statement>& stmts) const;
  Object EvalBangOperatorExpression(const Object& obj) const;
  Object EvalMinuxPrefixOperatorExpression(const Object& obj) const;
  Object EvalPrefixExpression(const std::string& op, const Object& obj) const;
  Object EvalInfixExpression(const Object& lhs, const std::string& op,
                             const Object& rhs) const;
  Object EvalIntegerInfixExpression(const Object& lhs, const std::string& op,
                                    const Object& rhs) const;

  static BoolObject kTrueObject;
  static BoolObject kFalseObject;
  static NullObject kNullObject;
};

}  // namespace monkey
