#pragma once

#include "monkey/ast.h"
#include "monkey/environment.h"
#include "monkey/object.h"
#include "monkey/timer.h"

namespace monkey {

class Evaluator {
 public:
  Object Evaluate(const Program& program, Environment& env) const;
  Object Evaluate(const AstNode& node, Environment& env) const;

  const auto& timers() const noexcept { return timers_; }

 private:
  Object EvalProgram(const Program& program, Environment& env) const;
  Object EvalIdentifier(const Identifier& ident, const Environment& env) const;
  Object EvalIfExpr(const IfExpr& expr, Environment& env) const;
  Object EvalBlockStmt(const BlockStmt& block, Environment& env) const;
  Object EvalDictLiteral(const DictLiteral& expr, Environment& env) const;

  std::vector<Object> EvalExprs(const std::vector<ExprNode>& exprs,
                                Environment& env) const;

  Object EvalBangOpExpr(const Object& obj) const;
  Object EvalMinuxPrefixOpExpr(const Object& obj) const;
  Object EvalPrefixExpr(const std::string& op, const Object& obj) const;
  Object EvalInfixExpr(const Object& lhs,
                       const std::string& op,
                       const Object& rhs) const;
  Object EvalIndexExpr(const Object& lhs, const Object& index) const;
  Object EvalArrayIndexExpr(const Object& obj, const Object& index) const;
  Object EvalDictIndexExpr(const Object& obj, const Object& key) const;
  Object EvalStrInfixExpr(const Object& lhs,
                          const std::string& op,
                          const Object& rhs) const;
  Object EvalIntInfixExpr(const Object& lhs,
                          const std::string& op,
                          const Object& rhs) const;
  Object EvalBoolInfixExpr(const Object& lhs,
                           const std::string& op,
                           const Object& rhs) const;
  Object ApplyFunc(const Object& fobj, const std::vector<Object>& args) const;

  mutable TimerManager timers_{"evaluator"};
};

}  // namespace monkey
