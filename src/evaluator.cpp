#include "monkey/evaluator.h"

#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

namespace {

const BoolObject kTrueObject{true};
const BoolObject kFalseObject{false};
const NullObject kNullObject{};

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

bool IsError(const Object& obj) noexcept {
  return obj.Type() == ObjectType::kError;
}

const std::string kUnknownOperator = "unknown operator";
const std::string kTypeMismatch = "type mismatch";

}  // namespace

Object Evaluator::Evaluate(const Program& node) const {
  return EvalProgram(node);
}

Object Evaluator::Evaluate(const Statement& stmt) const {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr());
    case NodeType::kBlockStmt:
      return EvalBlockStatment(*stmt.PtrCast<BlockStatement>());
    case NodeType::kReturnStmt: {
      auto obj = Evaluate(stmt.Expr());
      if (IsError(obj)) {
        return obj;
      }
      return ReturnObject{std::move(obj)};
    }
    case NodeType::kLetStmt: {
      const auto obj = Evaluate(stmt.Expr());
      if (IsError(obj)) {
        return obj;
      }

      return kNullObject;
    }
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
      const auto rhs = Evaluate(ptr->rhs);
      if (IsError(rhs)) {
        return rhs;
      }
      return EvalPrefixExpression(ptr->op, rhs);
    }
    case NodeType::kInfixExpr: {
      const auto* ptr = expr.PtrCast<InfixExpression>();
      const auto lhs = Evaluate(ptr->lhs);
      if (IsError(lhs)) {
        return lhs;
      }
      const auto rhs = Evaluate(ptr->rhs);
      if (IsError(rhs)) {
        return rhs;
      }
      return EvalInfixExpression(lhs, ptr->op, rhs);
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
    } else if (obj.Type() == ObjectType::kError) {
      return obj;
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
    return ErrorObject{
        fmt::format("{}: {}{}", kUnknownOperator, op, obj.Type())};
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
  } else if (lhs.Type() != rhs.Type()) {
    return ErrorObject(
        fmt::format("{}: {} {} {}", kTypeMismatch, lhs.Type(), op, rhs.Type()));
  } else {
    return ErrorObject{fmt::format("{}: {} {} {}", kUnknownOperator, lhs.Type(),
                                   op, rhs.Type())};
  }
}

Object Evaluator::EvalBlockStatment(const BlockStatement& block) const {
  Object obj{ObjectBase{}};

  for (const auto& stmt : block.statements) {
    obj = Evaluate(stmt);

    if (obj.Type() == ObjectType::kReturn || obj.Type() == ObjectType::kError) {
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
    return ErrorObject{fmt::format("{}: -{}", kUnknownOperator, obj.Type())};
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
    return ErrorObject{fmt::format("{}: {} {} {}", kUnknownOperator, lhs.Type(),
                                   op, rhs.Type())};
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
    return ErrorObject{fmt::format("{}: {} {} {}", kUnknownOperator, lhs.Type(),
                                   op, rhs.Type())};
  }
}

Object Evaluator::EvalIfExpression(const IfExpression& expr) const {
  const auto cond = Evaluate(expr.cond);
  if (IsError(cond)) {
    return cond;
  }

  if (IsTruthy(cond)) {
    return EvalBlockStatment(expr.true_block);
  } else if (!expr.false_block.empty()) {
    return EvalBlockStatment(expr.false_block);
  } else {
    return kNullObject;
  }
}

}  // namespace monkey
