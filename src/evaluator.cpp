#include "monkey/evaluator.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "monkey/builtin.h"

namespace monkey {

namespace {
// Error message prefix
const std::string kNotAFunc = "not a function";
const std::string kUnknownOp = "unknown operator";
const std::string kTypeMismatch = "type mismatch";
const std::string kIdentNotFound = "identifier not found";
const std::string kIndexOpNotSupported = "index operator not supported";
const BuiltinMap gBuiltins = MakeBuiltins();

// Some const objects
const Object kTrueObject = BoolObj(true);
const Object kFalseObject = BoolObj(false);
const Object kNullObject = NullObj();

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

Environment ExtendFunctionEnv(const FuncObject& func,
                              const std::vector<Object>& args) {
  auto env = MakeEnclosedEnv(func.env.get());

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

Object Evaluator::Evaluate(const AstNode& node, Environment& env) const {
  switch (node.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(GetExpr(node), env);
    case NodeType::kBlockStmt:
      return EvalBlockStmt(*node.PtrCast<BlockStmt>(), env);
    case NodeType::kReturnStmt: {
      auto obj = Evaluate(GetExpr(node), env);
      if (IsError(obj)) {
        return obj;
      }
      return ReturnObj(std::move(obj));
    }
    case NodeType::kLetStmt: {
      const auto obj = Evaluate(GetExpr(node), env);
      if (IsError(obj)) {
        return obj;
      }
      const auto* ptr = node.PtrCast<LetStmt>();
      return env.Set(ptr->name.String(), obj);
    }
    case NodeType::kIntLiteral: {
      return IntObj(node.PtrCast<IntLiteral>()->value);
    }
    case NodeType::kBoolLiteral: {
      return node.PtrCast<BoolLiteral>()->value ? kTrueObject : kFalseObject;
    }
    case NodeType::kStrLiteral: {
      return StrObj(node.PtrCast<StrLiteral>()->value);
    }
    case NodeType::kPrefixExpr: {
      const auto* pe_ptr = node.PtrCast<PrefixExpr>();
      const auto rhs = Evaluate(pe_ptr->rhs, env);
      if (IsError(rhs)) {
        return rhs;
      }
      return EvalPrefixExpr(pe_ptr->op, rhs);
    }
    case NodeType::kInfixExpr: {
      const auto* ie_ptr = node.PtrCast<InfixExpr>();
      const auto lhs = Evaluate(ie_ptr->lhs, env);
      if (IsError(lhs)) {
        return lhs;
      }
      const auto rhs = Evaluate(ie_ptr->rhs, env);
      if (IsError(rhs)) {
        return rhs;
      }
      return EvalInfixExpr(lhs, ie_ptr->op, rhs);
    }
    case NodeType::kIfExpr: {
      return EvalIfExpr(*node.PtrCast<IfExpr>(), env);
    }
    case NodeType::kIdentifier: {
      return EvalIdentifier(*node.PtrCast<Identifier>(), env);
    }
    case NodeType::kFnLiteral: {
      const auto* fn_ptr = node.PtrCast<FuncLiteral>();
      return FuncObj(
          {fn_ptr->params, fn_ptr->body, std::make_shared<Environment>(env)});
    }
    case NodeType::kArrayLiteral: {
      const auto* arr_ptr = node.PtrCast<ArrayLiteral>();
      auto elems = EvalExprs(arr_ptr->elements, env);

      if (elems.size() == 1 && IsError(elems.front())) {
        return elems.front();
      }

      return ArrayObj(std::move(elems));
    }
    case NodeType::kIndexExpr: {
      const auto* index_expr = node.PtrCast<IndexExpr>();

      const auto lhs = Evaluate(index_expr->lhs, env);
      if (IsError(lhs)) {
        return lhs;
      }
      const auto index = Evaluate(index_expr->index, env);
      if (IsError(index)) {
        return index;
      }
      return EvalIndexExpr(lhs, index);
    }
    case NodeType::kCallExpr: {
      const auto* ce_ptr = node.PtrCast<CallExpr>();

      const auto obj = Evaluate(ce_ptr->func, env);
      if (IsError(obj)) {
        return obj;
      }

      const auto args_obj = EvalExprs(ce_ptr->args, env);

      // Handle argument evaluation failure
      if (args_obj.size() == 1 && IsError(args_obj.front())) {
        return args_obj.front();
      }

      return ApplyFunc(obj, args_obj);
    }
    default:
      return NullObj();
  }
}

Object Evaluator::EvalProgram(const Program& program, Environment& env) const {
  Object obj = NullObj();

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
  const auto obj = env.Get(ident.value);
  if (obj.Ok()) {
    return obj;
  }

  const auto it = gBuiltins.find(ident.value);
  if (it != gBuiltins.end()) {
    return it->second;
  }

  return ErrorObj(fmt::format("{}: {}", kIdentNotFound, ident.value));
}

Object Evaluator::EvalPrefixExpr(const std::string& op,
                                 const Object& obj) const {
  if (op == "!") {
    return EvalBangOpExpr(obj);
  } else if (op == "-") {
    return EvalMinuxPrefixOpExpr(obj);
  } else {
    return ErrorObj(fmt::format("{}: {}{}", kUnknownOp, op, obj.Type()));
  }
}

Object Evaluator::EvalInfixExpr(const Object& lhs,
                                const std::string& op,
                                const Object& rhs) const {
  if (lhs.Type() == ObjectType::kInt && rhs.Type() == ObjectType::kInt) {
    return EvalIntInfixExpr(lhs, op, rhs);
  } else if (lhs.Type() == ObjectType::kBool &&
             rhs.Type() == ObjectType::kBool) {
    return EvalBoolInfixExpr(lhs, op, rhs);
  } else if (lhs.Type() == ObjectType::kStr && rhs.Type() == ObjectType::kStr) {
    return EvalStrInfixExpr(lhs, op, rhs);
  } else if (lhs.Type() != rhs.Type()) {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kTypeMismatch, lhs.Type(), op, rhs.Type()));
  } else {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
  }
}

Object Evaluator::EvalIndexExpr(const Object& lhs, const Object& index) const {
  if (lhs.Type() == ObjectType::kArray && index.Type() == ObjectType::kInt) {
    return EvalArrayIndexExpr(lhs, index);
  } else {
    return ErrorObj(fmt::format("{} {}", kIndexOpNotSupported, lhs.Type()));
  }
}

Object Evaluator::EvalArrayIndexExpr(const Object& array,
                                     const Object& index) const {
  const auto& arr = array.Cast<Array>();
  const auto idx = index.Cast<int64_t>();

  if (idx < 0 || idx >= arr.size()) {
    return kNullObject;
  }
  return arr[static_cast<size_t>(idx)];
}

Object Evaluator::EvalStrInfixExpr(const Object& lhs,
                                   const std::string& op,
                                   const Object& rhs) const {
  if (op != "+") {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
  }

  const auto lv = lhs.Cast<std::string>();
  const auto rv = rhs.Cast<std::string>();
  return StrObj(lv + rv);
}

Object Evaluator::EvalBlockStmt(const BlockStmt& block,
                                Environment& env) const {
  Object obj = NullObj();

  for (const auto& stmt : block.statements) {
    obj = Evaluate(stmt, env);

    if (obj.Type() == ObjectType::kReturn || obj.Type() == ObjectType::kError) {
      return obj;
    }
  }

  return obj;
}

std::vector<Object> Evaluator::EvalExprs(const std::vector<ExprNode>& exprs,
                                         Environment& env) const {
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

Object Evaluator::EvalBangOpExpr(const Object& obj) const {
  switch (obj.Type()) {
    case ObjectType::kBool:
      return obj.Cast<bool>() ? kFalseObject : kTrueObject;
    case ObjectType::kNull:
      return kTrueObject;
    default:
      return kFalseObject;
  }
}

Object Evaluator::EvalMinuxPrefixOpExpr(const Object& obj) const {
  if (obj.Type() != ObjectType::kInt) {
    return ErrorObj(fmt::format("{}: -{}", kUnknownOp, obj.Type()));
  }

  return IntObj(-obj.Cast<int64_t>());
}

Object Evaluator::EvalIntInfixExpr(const Object& lhs,
                                   const std::string& op,
                                   const Object& rhs) const {
  const auto lv = lhs.Cast<int64_t>();
  const auto rv = rhs.Cast<int64_t>();

  if (op == "+") {
    return IntObj(lv + rv);
  } else if (op == "-") {
    return IntObj(lv - rv);
  } else if (op == "*") {
    return IntObj(lv * rv);
  } else if (op == "/") {
    return IntObj(lv / rv);
  } else if (op == "==") {
    return BoolObj(lv == rv);
  } else if (op == "!=") {
    return BoolObj(lv != rv);
  } else if (op == ">") {
    return BoolObj(lv > rv);
  } else if (op == ">=") {
    return BoolObj(lv >= rv);
  } else if (op == "<") {
    return BoolObj(lv < rv);
  } else if (op == "<=") {
    return BoolObj(lv <= rv);
  } else {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
  }
}

Object Evaluator::EvalBoolInfixExpr(const Object& lhs,
                                    const std::string& op,
                                    const Object& rhs) const {
  const auto lv = lhs.Cast<bool>();
  const auto rv = rhs.Cast<bool>();
  if (op == "==") {
    return BoolObj(lv == rv);
  } else if (op == "!=") {
    return BoolObj(lv != rv);
  } else {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
  }
}

Object Evaluator::ApplyFunc(const Object& obj,
                            const std::vector<Object>& args) const {
  switch (obj.Type()) {
    case ObjectType::kFunc: {
      const auto& fn_obj = obj.Cast<FuncObject>();
      auto fn_env = ExtendFunctionEnv(fn_obj, args);
      const auto ret_obj = EvalBlockStmt(fn_obj.body, fn_env);
      return UnwrapReturn(ret_obj);
    }
    case ObjectType::kBuiltinFunc: {
      const auto& fn_obj = obj.Cast<BuiltinFunc>();
      return fn_obj(args);
    }
    default:
      return ErrorObj(fmt::format("{}: {}", kNotAFunc, obj.Type()));
  }
}

Object Evaluator::EvalIfExpr(const IfExpr& expr, Environment& env) const {
  const auto cond = Evaluate(expr.cond, env);
  if (IsError(cond)) {
    return cond;
  }

  if (IsTruthy(cond)) {
    return EvalBlockStmt(expr.true_block, env);
  } else if (!expr.false_block.empty()) {
    return EvalBlockStmt(expr.false_block, env);
  } else {
    return kNullObject;
  }
}

}  // namespace monkey
