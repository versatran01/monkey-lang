#pragma once

#include "monkey/ast.h"
#include "monkey/environment.h"
#include "monkey/object.h"

namespace monkey {

class Evaluator {
 public:
  Object Evaluate(const Program& program, Environment& env) const;
  Object Evaluate(const StmtNode& stmt, Environment& env) const;
  Object Evaluate(const ExprNode& expr, Environment& env) const;

 private:
  Object EvalProgram(const Program& program, Environment& env) const;
  Object EvalIdentifier(const Identifier& ident, const Environment& env) const;
  Object EvalIfExpression(const IfExpr& expr, Environment& env) const;
  Object EvalBlockStatment(const BlockStmt& block, Environment& env) const;
  std::vector<Object> EvalExpressions(const std::vector<ExprNode>& exprs,
                                      Environment& env) const;

  Object EvalBangOperatorExpression(const Object& obj) const;
  Object EvalMinuxPrefixOperatorExpression(const Object& obj) const;
  Object EvalPrefixExpression(const std::string& op, const Object& obj) const;
  Object EvalInfixExpression(const Object& lhs,
                             const std::string& op,
                             const Object& rhs) const;
  Object EvalIntInfixExpression(const Object& lhs,
                                const std::string& op,
                                const Object& rhs) const;
  Object EvalBoolInfixExpression(const Object& lhs,
                                 const std::string& op,
                                 const Object& rhs) const;
  Object ApplyFunction(const Object& fobj,
                       const std::vector<Object>& args) const;
};

}  // namespace monkey
