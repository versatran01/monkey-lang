#include "monkey/evaluator.h"

#include <glog/logging.h>

namespace monkey {

NullObject Evaluator::kNullObject = NullObject{};
BoolObject Evaluator::kTrueObject = BoolObject{true};
BoolObject Evaluator::kFalseObject = BoolObject{false};

Object Evaluator::Evaluate(const Program& node) const {
  return EvalStatements(node.statements);
}

Object Evaluator::Evaluate(const Statement& stmt) const {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr());
    default:
      return ObjectBase{};
  }
}

Object Evaluator::Evaluate(const Expression& expr) const {
  switch (expr.Type()) {
    case NodeType::kIntLiteral: {
      const auto* ptr = static_cast<IntegerLiteral*>(expr.Ptr());
      return IntObject{ptr->value};
    }
    case NodeType::kBoolLiteral: {
      const auto* ptr = static_cast<BooleanLiteral*>(expr.Ptr());
      return ptr->value ? kTrueObject : kFalseObject;
    }
    case NodeType::kPrefixExpr: {
      const auto* ptr = static_cast<PrefixExpression*>(expr.Ptr());
      return EvalPrefixExpression(ptr->op, Evaluate(ptr->rhs));
    }
    default:
      return ObjectBase{};
  }
}

Object Evaluator::EvalStatements(const std::vector<Statement>& stmts) const {
  Object obj{ObjectBase{}};
  for (const auto& stmt : stmts) {
    obj = Evaluate(stmt);
  }
  return obj;
}

Object Evaluator::EvalPrefixExpression(const std::string& op,
                                       const Object& obj) const {
  if (op == "!") {
    return EvalBangOperatorExpression(obj);
  } else if (op == "-") {
    return EvalMinuxPrefixOperatorExpression(obj);
  } else {
    return kNullObject;
  }
}

Object Evaluator::EvalBangOperatorExpression(const Object& obj) const {
  switch (obj.Type()) {
    case ObjectType::kBool: {
      const auto* ptr = static_cast<BoolObject*>(obj.Ptr());
      return ptr->value ? kFalseObject : kTrueObject;
    }
    case ObjectType::kNull:
      return kTrueObject;
    default:
      return kFalseObject;
  }
}

Object Evaluator::EvalMinuxPrefixOperatorExpression(const Object& obj) const {
  if (obj.Type() != ObjectType::kInt) {
    return kNullObject;
  }

  const auto* ptr = static_cast<IntObject*>(obj.Ptr());
  return IntObject{-(ptr->value)};
}

}  // namespace monkey
