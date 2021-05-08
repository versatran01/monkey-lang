#include "monkey/evaluator.h"

#include <glog/logging.h>

namespace monkey {

NullObject Evaluator::kNullObject = NullObject{};
BoolObject Evaluator::kTrueObject = BoolObject{true};
BoolObject Evaluator::kFalseObject = BoolObject{false};

bool IsTruthy(const Object& obj) {
  switch (obj.Type()) {
    case ObjectType::kNull:
      return false;
    case ObjectType::kBool: {
      const auto* ptr = static_cast<BoolObject*>(obj.Ptr());
      return ptr->value;
    }
    default:
      return true;
  }
}

Object Evaluator::Evaluate(const Program& node) const {
  return EvalStatements(node.statements);
}

Object Evaluator::Evaluate(const Statement& stmt) const {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr());
    case NodeType::kBlockStmt: {
      const auto* ptr = static_cast<BlockStatement*>(stmt.Ptr());
      return EvalStatements(ptr->statements);
    }
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
    case NodeType::kIfExpr: {
      const auto* ptr = static_cast<IfExpression*>(expr.Ptr());
      return EvalIfExpression(*ptr);
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
    const auto* lp = static_cast<IntObject*>(lhs.Ptr());
    const auto* rp = static_cast<IntObject*>(rhs.Ptr());
    return EvalIntInfixExpression(*lp, op, *rp);
  } else if (lhs.Type() == ObjectType::kBool &&
             rhs.Type() == ObjectType::kBool) {
    const auto* lp = static_cast<BoolObject*>(lhs.Ptr());
    const auto* rp = static_cast<BoolObject*>(rhs.Ptr());
    return EvalBoolInfixExpression(*lp, op, *rp);
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

Object Evaluator::EvalIntInfixExpression(const IntObject& lhs,
                                         const std::string& op,
                                         const IntObject& rhs) const {
  const auto lv = lhs.value;
  const auto rv = rhs.value;

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

Object Evaluator::EvalBoolInfixExpression(const BoolObject& lhs,
                                          const std::string& op,
                                          const BoolObject& rhs) const {
  if (op == "==") {
    return BoolObject{lhs.value == rhs.value};
  } else if (op == "!=") {
    return BoolObject{lhs.value != rhs.value};
  } else {
    return kNullObject;
  }
}

Object Evaluator::EvalIfExpression(const IfExpression& expr) const {
  const auto cond = Evaluate(expr.cond);
  if (IsTruthy(cond)) {
    return EvalStatements(expr.true_block.statements);
  } else if (!expr.false_block.empty()) {
    return EvalStatements(expr.false_block.statements);
  } else {
    return kNullObject;
  }
}

}  // namespace monkey
