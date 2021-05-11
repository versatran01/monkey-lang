#include "monkey/evaluator.h"

#include <absl/strings/str_join.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace monkey {

namespace {

const Object kTrueObject = BoolObject(true);
const Object kFalseObject = BoolObject(false);
const Object kNullObject = NullObject();

bool IsTruthy(const Object& obj) {
  switch (obj.Type()) {
    case ObjectType::kNull:
      return false;
    case ObjectType::kBool:
      return obj.Cast<bool>();
    default:
      return true;
  }
}

bool IsError(const Object& obj) noexcept {
  return obj.Type() == ObjectType::kError;
}

const std::string kUnknownOperator = "unknown operator";
const std::string kTypeMismatch = "type mismatch";
const std::string kIdentifierNotFound = "identifier not found";
const std::string kNotAFunction = "not a function";

Environment ExtendFunctionEnv(const FnObject& func,
                              const std::vector<Object>& args) {
  auto env = MakeEnclosedEnv(*func.env);

  for (size_t i = 0; i < func.params.size(); ++i) {
    env.Set(func.params[i].value, args[i]);
  }

  return env;
}

Object UnwrapReturn(const Object& obj) {
  if (obj.Type() == ObjectType::kReturn) {
    return obj.Cast<Object>();
  }
  return obj;
}

}  // namespace

Object Evaluator::Evaluate(const Program& program, Environment& env) const {
  return EvalProgram(program, env);
}

Object Evaluator::Evaluate(const Statement& stmt, Environment& env) const {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr(), env);
    case NodeType::kBlockStmt:
      return EvalBlockStatment(*stmt.PtrCast<BlockStatement>(), env);
    case NodeType::kReturnStmt: {
      auto obj = Evaluate(stmt.Expr(), env);
      if (IsError(obj)) {
        return obj;
      }
      return ReturnObject(std::move(obj));
    }
    case NodeType::kLetStmt: {
      const auto obj = Evaluate(stmt.Expr(), env);
      if (IsError(obj)) {
        return obj;
      }
      return env.Set(stmt.PtrCast<LetStatement>()->name.String(), obj);
    }
    default:
      return NullObject();
  }
}

Object Evaluator::Evaluate(const Expression& expr, Environment& env) const {
  switch (expr.Type()) {
    case NodeType::kIntLiteral: {
      return IntObject(expr.PtrCast<IntLiteral>()->value);
    }
    case NodeType::kBoolLiteral: {
      return expr.PtrCast<BoolLiteral>()->value ? kTrueObject : kFalseObject;
    }
    case NodeType::kPrefixExpr: {
      const auto* pe_ptr = expr.PtrCast<PrefixExpression>();
      const auto rhs = Evaluate(pe_ptr->rhs, env);
      if (IsError(rhs)) {
        return rhs;
      }
      return EvalPrefixExpression(pe_ptr->op, rhs);
    }
    case NodeType::kInfixExpr: {
      const auto* ie_ptr = expr.PtrCast<InfixExpression>();
      const auto lhs = Evaluate(ie_ptr->lhs, env);
      if (IsError(lhs)) {
        return lhs;
      }
      const auto rhs = Evaluate(ie_ptr->rhs, env);
      if (IsError(rhs)) {
        return rhs;
      }
      return EvalInfixExpression(lhs, ie_ptr->op, rhs);
    }
    case NodeType::kIfExpr: {
      return EvalIfExpression(*expr.PtrCast<IfExpression>(), env);
    }
    case NodeType::kIdentifier: {
      return EvalIdentifier(*expr.PtrCast<Identifier>(), env);
    }
    case NodeType::kFnLiteral: {
      const auto* fn_ptr = expr.PtrCast<FunctionLiteral>();
      return FunctionObject({fn_ptr->params, fn_ptr->body, &env});
    }
    case NodeType::kCallExpr: {
      LOG(INFO) << "Eval Call Expr";
      const auto* ce_ptr = expr.PtrCast<CallExpression>();

      const auto obj = Evaluate(ce_ptr->func, env);
      if (IsError(obj)) {
        return obj;
      }

      const auto args_obj = EvalExpressions(ce_ptr->args, env);

      // Handle argument evaluation failure
      if (args_obj.size() == 1 && IsError(args_obj.front())) {
        return args_obj.front();
      }

      return ApplyFunction(obj, args_obj);
    }
    default:
      CHECK(false) << "Should not reach";
      return NullObject();
  }
}

Object Evaluator::EvalProgram(const Program& program, Environment& env) const {
  Object obj = NullObject();

  for (const auto& stmt : program.statements) {
    obj = Evaluate(stmt, env);

    if (obj.Type() == ObjectType::kReturn) {
      return obj.Cast<Object>();
    } else if (obj.Type() == ObjectType::kError) {
      return obj;
    }
  }

  return obj;
}

Object Evaluator::EvalIdentifier(const Identifier& ident,
                                 const Environment& env) const {
  const auto* obj_ptr = env.Get(ident.value);
  if (obj_ptr == nullptr) {
    return ErrorObject(fmt::format("{}: {}", kIdentifierNotFound, ident.value));
  }
  return *obj_ptr;
}

Object Evaluator::EvalPrefixExpression(const std::string& op,
                                       const Object& obj) const {
  if (op == "!") {
    return EvalBangOperatorExpression(obj);
  } else if (op == "-") {
    return EvalMinuxPrefixOperatorExpression(obj);
  } else {
    return ErrorObject(
        fmt::format("{}: {}{}", kUnknownOperator, op, obj.Type()));
  }
}

Object Evaluator::EvalInfixExpression(const Object& lhs,
                                      const std::string& op,
                                      const Object& rhs) const {
  if (lhs.Type() == ObjectType::kInt && rhs.Type() == ObjectType::kInt) {
    return EvalIntInfixExpression(lhs, op, rhs);
  } else if (lhs.Type() == ObjectType::kBool &&
             rhs.Type() == ObjectType::kBool) {
    return EvalBoolInfixExpression(lhs, op, rhs);
  } else if (lhs.Type() != rhs.Type()) {
    return ErrorObject(
        fmt::format("{}: {} {} {}", kTypeMismatch, lhs.Type(), op, rhs.Type()));
  } else {
    return ErrorObject(fmt::format(
        "{}: {} {} {}", kUnknownOperator, lhs.Type(), op, rhs.Type()));
  }
}

Object Evaluator::EvalBlockStatment(const BlockStatement& block,
                                    Environment& env) const {
  Object obj = NullObject();

  for (const auto& stmt : block.statements) {
    obj = Evaluate(stmt, env);

    if (obj.Type() == ObjectType::kReturn || obj.Type() == ObjectType::kError) {
      return obj;
    }
  }

  return obj;
}

std::vector<Object> Evaluator::EvalExpressions(
    const std::vector<Expression>& exprs, Environment& env) const {
  std::vector<Object> objs;

  for (const auto& expr : exprs) {
    auto obj = Evaluate(expr, env);
    if (IsError(obj)) {
      return {obj};
    }
    objs.push_back(std::move(obj));
  }

  return objs;
}

Object Evaluator::EvalBangOperatorExpression(const Object& obj) const {
  switch (obj.Type()) {
    case ObjectType::kBool:
      return obj.Cast<bool>() ? kFalseObject : kTrueObject;
    case ObjectType::kNull:
      return kTrueObject;
    default:
      return kFalseObject;
  }
}

Object Evaluator::EvalMinuxPrefixOperatorExpression(const Object& obj) const {
  if (obj.Type() != ObjectType::kInt) {
    return ErrorObject(fmt::format("{}: -{}", kUnknownOperator, obj.Type()));
  }

  return IntObject(-obj.Cast<int64_t>());
}

Object Evaluator::EvalIntInfixExpression(const Object& lhs,
                                         const std::string& op,
                                         const Object& rhs) const {
  const auto lv = lhs.Cast<int64_t>();
  const auto rv = rhs.Cast<int64_t>();

  if (op == "+") {
    return IntObject(lv + rv);
  } else if (op == "-") {
    return IntObject(lv - rv);
  } else if (op == "*") {
    return IntObject(lv * rv);
  } else if (op == "/") {
    return IntObject(lv / rv);
  } else if (op == "==") {
    return BoolObject(lv == rv);
  } else if (op == "!=") {
    return BoolObject(lv != rv);
  } else if (op == ">") {
    return BoolObject(lv > rv);
  } else if (op == ">=") {
    return BoolObject(lv >= rv);
  } else if (op == "<") {
    return BoolObject(lv < rv);
  } else if (op == "<=") {
    return BoolObject(lv <= rv);
  } else {
    return ErrorObject(fmt::format(
        "{}: {} {} {}", kUnknownOperator, lhs.Type(), op, rhs.Type()));
  }
}

Object Evaluator::EvalBoolInfixExpression(const Object& lhs,
                                          const std::string& op,
                                          const Object& rhs) const {
  const auto lv = lhs.Cast<bool>();
  const auto rv = rhs.Cast<bool>();
  if (op == "==") {
    return BoolObject(lv == rv);
  } else if (op == "!=") {
    return BoolObject(lv != rv);
  } else {
    return ErrorObject(fmt::format(
        "{}: {} {} {}", kUnknownOperator, lhs.Type(), op, rhs.Type()));
  }
}

Object Evaluator::ApplyFunction(const Object& obj,
                                const std::vector<Object>& args) const {
  LOG(INFO) << "[ApplyFn] " << obj.Inspect();
  if (obj.Type() != ObjectType::kFunction) {
    return ErrorObject(fmt::format("{}: {}", kNotAFunction, obj.Type()));
  }

  const auto& fn_obj = obj.Cast<FnObject>();
  auto fn_env = ExtendFunctionEnv(fn_obj, args);
  const auto ret_obj = EvalBlockStatment(fn_obj.body, fn_env);
  return UnwrapReturn(ret_obj);
}

Object Evaluator::EvalIfExpression(const IfExpression& expr,
                                   Environment& env) const {
  const auto cond = Evaluate(expr.cond, env);
  if (IsError(cond)) {
    return cond;
  }

  if (IsTruthy(cond)) {
    return EvalBlockStatment(expr.true_block, env);
  } else if (!expr.false_block.empty()) {
    return EvalBlockStatment(expr.false_block, env);
  } else {
    return kNullObject;
  }
}

}  // namespace monkey
