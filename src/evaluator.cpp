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
    case NodeType::kInfixExpr: {
      const auto* ptr = static_cast<InfixExpression*>(expr.Ptr());
      return EvalInfixExpression(Evaluate(ptr->lhs), ptr->op,
                                 Evaluate(ptr->rhs));
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

Object Evaluator::EvalInfixExpression(const Object& lhs, const std::string& op,
                                      const Object& rhs) const {
  if (lhs.Type() == ObjectType::kInt && rhs.Type() == ObjectType::kInt) {
    return EvalIntegerInfixExpression(lhs, op, rhs);
  } else if (lhs.Type() == ObjectType::kBool &&
             rhs.Type() == ObjectType::kBool) {
    const auto* lp = static_cast<BoolObject*>(lhs.Ptr());
    const auto* rp = static_cast<BoolObject*>(rhs.Ptr());
    if (op == "==") {
      return BoolObject{lp->value == rp->value};
    } else if (op == "!=") {
      return BoolObject{lp->value != rp->value};
    } else {
      return kNullObject;
    }
  } else {
    return kNullObject;
  }
}

Object Evaluator::EvalIntegerInfixExpression(const Object& lhs,
                                             const std::string& op,
                                             const Object& rhs) const {
  const auto* lp = static_cast<IntObject*>(lhs.Ptr());
  const auto* rp = static_cast<IntObject*>(rhs.Ptr());
  const auto lv = lp->value;
  const auto rv = rp->value;

  if (op == "+") {
    return IntObject{lv + rv};
  } else if (op == "-") {
    return IntObject{lv - rv};
  } else if (op == "*") {
    return IntObject{lv * rv};
  } else if (op == "/") {
    return IntObject{lv / rv};
  } else if (op == "==") {
    return BoolObject{lv == rv};
  } else if (op == "!=") {
    return BoolObject{lv != rv};
  } else if (op == ">") {
    return BoolObject{lv > rv};
  } else if (op == ">=") {
    return BoolObject{lv >= rv};
  } else if (op == "<") {
    return BoolObject{lv < rv};
  } else if (op == "<=") {
    return BoolObject{lv <= rv};
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
