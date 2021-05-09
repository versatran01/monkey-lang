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
  Object EvalProgram(const Program& program) const;
  Object EvalBlockStatment(const BlockStatement& block) const;
  Object EvalIfExpression(const IfExpression& expr) const;

  Object EvalBangOperatorExpression(const Object& obj) const;
  Object EvalMinuxPrefixOperatorExpression(const Object& obj) const;
  Object EvalPrefixExpression(const std::string& op, const Object& obj) const;
  Object EvalInfixExpression(const Object& lhs, const std::string& op,
                             const Object& rhs) const;
  Object EvalIntInfixExpression(const IntObject& lhs, const std::string& op,
                                const IntObject& rhs) const;
  Object EvalBoolInfixExpression(const BoolObject& lhs, const std::string& op,
                                 const BoolObject& rhs) const;
};

}  // namespace monkey
