#include "monkey/evaluator.h"

#include <glog/logging.h>

namespace monkey {

NullObject Evaluator::kNullObject = NullObject{};
BoolObject Evaluator::kTrueObject = BoolObject{true};
BoolObject Evaluator::kFalseObject = BoolObject{false};

Object Evaluator::Evaluate(const Program& node) {
  return EvalStatements(node.statements);
}

Object Evaluator::EvalStatements(const std::vector<Statement>& stmts) {
  Object obj{ObjectBase{}};
  for (const auto& stmt : stmts) {
    obj = Evaluate(stmt);
  }
  return obj;
}

Object Evaluator::Evaluate(const Statement& stmt) {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr());
    default:
      return ObjectBase{};
  }
}

Object Evaluator::Evaluate(const Expression& expr) {
  switch (expr.Type()) {
    case NodeType::kIntLiteral: {
      const auto* ptr = static_cast<IntegerLiteral*>(expr.Ptr());
      IntObject obj;
      obj.value = ptr->value;
      return obj;
    }
    case NodeType::kBoolLiteral: {
      const auto* ptr = static_cast<IntegerLiteral*>(expr.Ptr());
      return ptr->value ? kTrueObject : kFalseObject;
    }
    default:
      return ObjectBase{};
  }
}

}  // namespace monkey
