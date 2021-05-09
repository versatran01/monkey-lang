#include "monkey/evaluator.h"

#include <glog/logging.h>

namespace monkey {

namespace {
const BoolObject kTrueObject{true};
const BoolObject kFalseObject{false};
const NullObject kNullObject{};
}  // namespace

bool IsTruthy(const Object& obj) {
  switch (obj.Type()) {
    case ObjectType::kNull:
      return false;
    case ObjectType::kBool:
      return obj.PtrCast<BoolObject>()->value;
    default:
      return true;
  }
}

Object Evaluator::Evaluate(const Program& node) const {
  return EvalProgram(node);
}

Object Evaluator::Evaluate(const Statement& stmt) const {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr());
    case NodeType::kBlockStmt:
      return EvalBlockStatment(*stmt.PtrCast<BlockStatement>());
    case NodeType::kReturnStmt:
      return ReturnObject{Evaluate(stmt.Expr())};
    default:
      return ObjectBase{};
  }
}

Object Evaluator::Evaluate(const Expression& expr) const {
  switch (expr.Type()) {
    case NodeType::kIntLiteral:
      return IntObject{expr.PtrCast<IntLiteral>()->value};
    case NodeType::kBoolLiteral:
      return expr.PtrCast<BoolLiteral>()->value ? kTrueObject : kFalseObject;
    case NodeType::kPrefixExpr: {
      const auto* ptr = expr.PtrCast<PrefixExpression>();
      return EvalPrefixExpression(ptr->op, Evaluate(ptr->rhs));
    }
    case NodeType::kInfixExpr: {
      const auto* ptr = expr.PtrCast<InfixExpression>();
      return EvalInfixExpression(Evaluate(ptr->lhs), ptr->op,
                                 Evaluate(ptr->rhs));
    }
    case NodeType::kIfExpr:
      return EvalIfExpression(*expr.PtrCast<IfExpression>());
    default:
      return ObjectBase{};
  }
}

Object Evaluator::EvalProgram(const Program& program) const {
  Object obj{ObjectBase{}};

  for (const auto& stmt : program.statements) {
    obj = Evaluate(stmt);

    if (obj.Type() == ObjectType::kReturn) {
      return obj.PtrCast<ReturnObject>()->value;
    }
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
    return EvalIntInfixExpression(*lhs.PtrCast<IntObject>(), op,
                                  *rhs.PtrCast<IntObject>());
  } else if (lhs.Type() == ObjectType::kBool &&
             rhs.Type() == ObjectType::kBool) {
    return EvalBoolInfixExpression(*lhs.PtrCast<BoolObject>(), op,
                                   *rhs.PtrCast<BoolObject>());
  } else {
    return kNullObject;
  }
}

Object Evaluator::EvalBlockStatment(const BlockStatement& block) const {
  Object obj{ObjectBase{}};

  for (const auto& stmt : block.statements) {
    obj = Evaluate(stmt);

    if (obj.Type() == ObjectType::kReturn) {
      return obj;
    }
  }

  return obj;
}

Object Evaluator::EvalBangOperatorExpression(const Object& obj) const {
  switch (obj.Type()) {
    case ObjectType::kBool:
      return obj.PtrCast<BoolObject>()->value ? kFalseObject : kTrueObject;
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

  return IntObject{-(obj.PtrCast<IntObject>()->value)};
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
    return EvalBlockStatment(expr.true_block);
  } else if (!expr.false_block.empty()) {
    return EvalBlockStatment(expr.false_block);
  } else {
    return kNullObject;
  }
}

}  // namespace monkey
