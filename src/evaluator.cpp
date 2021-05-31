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

Environment ExtendFunctionEnv(const FuncObject& func,
                              const std::vector<Object>& args) {
  Environment env{func.env.get()};

  for (size_t i = 0; i < func.params.size(); ++i) {
    env.Set(func.params[i].value, args[i]);
  }

  return env;
}

Object UnwrapReturn(const Object& obj) {
  return obj.Type() == ObjectType::kReturn ? obj.Cast<Object>() : obj;
}

}  // namespace

Object Evaluator::Evaluate(const Program& program, Environment& env) const {
  auto _ = timers_.Scoped("EvaluateProgram");
  return EvalProgram(program, env);
}

Object Evaluator::Evaluate(const AstNode& node, Environment& env) const {
  switch (node.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(GetExpr(node), env);
    case NodeType::kBlockStmt:
      return EvalBlockStmt(*node.PtrCast<BlockStmt>(), env);
    case NodeType::kReturnStmt: {
      const auto obj = Evaluate(GetExpr(node), env);
      if (IsObjError(obj)) return obj;
      return ReturnObj(obj);
    }
    case NodeType::kLetStmt: {
      const auto obj = Evaluate(GetExpr(node), env);
      if (IsObjError(obj)) return obj;
      return env.Set(node.PtrCast<LetStmt>()->name.String(), obj);
    }
    case NodeType::kIntLiteral: {
      return ToIntObj(node);
    }
    case NodeType::kBoolLiteral: {
      return ToBoolObj(node);
    }
    case NodeType::kStrLiteral: {
      return ToStrObj(node);
    }
    case NodeType::kDictLiteral: {
      return EvalDictLiteral(*node.PtrCast<DictLiteral>(), env);
    }
    case NodeType::kPrefixExpr: {
      const auto* pe_ptr = node.PtrCast<PrefixExpr>();
      const auto rhs = Evaluate(pe_ptr->rhs, env);
      if (IsObjError(rhs)) return rhs;
      return EvalPrefixExpr(pe_ptr->op, rhs);
    }
    case NodeType::kInfixExpr: {
      const auto* ie_ptr = node.PtrCast<InfixExpr>();
      const auto lhs = Evaluate(ie_ptr->lhs, env);
      if (IsObjError(lhs)) return lhs;

      const auto rhs = Evaluate(ie_ptr->rhs, env);
      if (IsObjError(rhs)) return rhs;

      return EvalInfixExpr(lhs, ie_ptr->op, rhs);
    }
    case NodeType::kIfExpr: {
      return EvalIfExpr(*node.PtrCast<IfExpr>(), env);
    }
    case NodeType::kIdentifier: {
      return EvalIdentifier(*node.PtrCast<Identifier>(), env);
    }
    case NodeType::kFuncLiteral: {
      const auto* ptr = node.PtrCast<FuncLiteral>();
      return FuncObj(
          {ptr->params, ptr->body, std::make_shared<Environment>(env)});
    }
    case NodeType::kArrayLiteral: {
      const auto* ptr = node.PtrCast<ArrayLiteral>();
      auto elems = EvalExprs(ptr->elements, env);

      if (elems.size() == 1 && IsObjError(elems.front())) {
        return elems.front();
      }

      return ArrayObj(std::move(elems));
    }
    case NodeType::kIndexExpr: {
      const auto* expr = node.PtrCast<IndexExpr>();

      const auto lhs = Evaluate(expr->lhs, env);
      if (IsObjError(lhs)) return lhs;

      const auto index = Evaluate(expr->index, env);
      if (IsObjError(index)) return index;

      return EvalIndexExpr(lhs, index);
    }
    case NodeType::kCallExpr: {
      // Skip evaluation for quote
      const auto* ptr = node.PtrCast<CallExpr>();
      if (ptr->func.TokenLiteral() == "quote") {
        CHECK_GT(ptr->args.size(), 0);
        return QuoteObj(ptr->args.front());
      }

      const auto func = Evaluate(ptr->func, env);
      if (IsObjError(func)) return func;

      const auto args = EvalExprs(ptr->args, env);

      // Handle argument evaluation failure
      if (args.size() == 1 && IsObjError(args.front())) {
        return args.front();
      }

      return ApplyFunc(func, args);
    }
    default:
      return NullObj();
  }
}

Object Evaluator::EvalProgram(const Program& program, Environment& env) const {
  Object obj = NullObj();

  for (const auto& stmt : program.statements) {
    obj = Evaluate(stmt, env);
    if (obj.Type() == ObjectType::kReturn) return obj.Cast<Object>();
    if (obj.Type() == ObjectType::kError) return obj;
  }

  return obj;
}

Object Evaluator::EvalIdentifier(const Identifier& ident,
                                 const Environment& env) const {
  const auto obj = env.Get(ident.value);
  if (obj.Ok()) return obj;

  const auto it = gBuiltins.find(ident.value);
  if (it != gBuiltins.end()) return it->second;

  return ErrorObj(fmt::format("{}: {}", kIdentNotFound, ident.value));
}

Object Evaluator::EvalPrefixExpr(const std::string& op,
                                 const Object& obj) const {
  if (op == "!") return EvalBangOpExpr(obj);
  if (op == "-") return EvalMinusPrefixOpExpr(obj);
  return ErrorObj(fmt::format("{}: {}{}", kUnknownOp, op, obj.Type()));
}

Object Evaluator::EvalInfixExpr(const Object& lhs,
                                const std::string& op,
                                const Object& rhs) const {
  if (ObjOfSameType(ObjectType::kInt, lhs, rhs)) {
    return EvalIntInfixExpr(lhs, op, rhs);
  }
  if (ObjOfSameType(ObjectType::kBool, lhs, rhs)) {
    return EvalBoolInfixExpr(lhs, op, rhs);
  }
  if (ObjOfSameType(ObjectType::kStr, lhs, rhs)) {
    return EvalStrInfixExpr(lhs, op, rhs);
  }
  if (lhs.Type() != rhs.Type()) {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kTypeMismatch, lhs.Type(), op, rhs.Type()));
  }
  return ErrorObj(
      fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
}

Object Evaluator::EvalIndexExpr(const Object& lhs, const Object& index) const {
  if (lhs.Type() == ObjectType::kArray && index.Type() == ObjectType::kInt) {
    return EvalArrayIndexExpr(lhs, index);
  }

  if (lhs.Type() == ObjectType::kDict) {
    return EvalDictIndexExpr(lhs, index);
  }

  return ErrorObj(fmt::format("{} {}", kIndexOpNotSupported, lhs.Type()));
}

Object Evaluator::EvalArrayIndexExpr(const Object& obj,
                                     const Object& index) const {
  CHECK_EQ(obj.Type(), ObjectType::kArray);
  const auto& arr = obj.Cast<Array>();

  CHECK_EQ(index.Type(), ObjectType::kInt);
  const auto idx = static_cast<size_t>(index.Cast<IntType>());

  if (idx < size_t{0} || idx >= arr.size()) return kNullObject;
  return arr[idx];
}

Object Evaluator::EvalDictIndexExpr(const Object& obj,
                                    const Object& key) const {
  CHECK_EQ(obj.Type(), ObjectType::kDict);
  const auto& dict = obj.Cast<Dict>();

  if (!IsObjHashable(key)) {
    return ErrorObj(fmt::format("unusable as dict key: {}", key.Type()));
  }

  // get the value
  const auto it = dict.find(key);
  if (it == dict.end()) return kNullObject;
  return it->second;
}

Object Evaluator::EvalStrInfixExpr(const Object& lhs,
                                   const std::string& op,
                                   const Object& rhs) const {
  if (op != "+") {
    return ErrorObj(
        fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
  }

  CHECK_EQ(lhs.Type(), ObjectType::kStr);
  CHECK_EQ(rhs.Type(), ObjectType::kStr);

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

Object Evaluator::EvalDictLiteral(const DictLiteral& expr,
                                  Environment& env) const {
  Dict dict;

  for (const auto& [k, v] : expr.pairs) {
    const auto key = Evaluate(k, env);

    if (IsObjError(key)) return key;

    // Check if key is valid
    if (!IsObjHashable(key)) {
      return ErrorObj(fmt::format("unusable as dict key: {}", key.Type()));
    }

    const auto val = Evaluate(v, env);
    if (IsObjError(val)) return val;

    dict[key] = val;
  }

  return DictObj(std::move(dict));
}

std::vector<Object> Evaluator::EvalExprs(const std::vector<ExprNode>& exprs,
                                         Environment& env) const {
  std::vector<Object> objs;

  for (const auto& expr : exprs) {
    const auto obj = Evaluate(expr, env);
    if (IsObjError(obj)) return {obj};

    objs.push_back(obj);
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

Object Evaluator::EvalMinusPrefixOpExpr(const Object& obj) const {
  if (obj.Type() == ObjectType::kInt) {
    return IntObj(-obj.Cast<IntType>());
  }
  return ErrorObj(fmt::format("{}: -{}", kUnknownOp, obj.Type()));
}

Object Evaluator::EvalIntInfixExpr(const Object& lhs,
                                   const std::string& op,
                                   const Object& rhs) const {
  const auto lv = lhs.Cast<IntType>();
  const auto rv = rhs.Cast<IntType>();

  if (op == "+") return IntObj(lv + rv);
  if (op == "-") return IntObj(lv - rv);
  if (op == "*") return IntObj(lv * rv);
  if (op == "/") return IntObj(lv / rv);
  if (op == "==") return BoolObj(lv == rv);
  if (op == "!=") return BoolObj(lv != rv);
  if (op == ">") return BoolObj(lv > rv);
  if (op == ">=") return BoolObj(lv >= rv);
  if (op == "<") return BoolObj(lv < rv);
  if (op == "<=") return BoolObj(lv <= rv);
  return ErrorObj(
      fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
}

Object Evaluator::EvalBoolInfixExpr(const Object& lhs,
                                    const std::string& op,
                                    const Object& rhs) const {
  const auto lv = lhs.Cast<bool>();
  const auto rv = rhs.Cast<bool>();
  if (op == "==") return BoolObj(lv == rv);
  if (op == "!=") return BoolObj(lv != rv);
  return ErrorObj(
      fmt::format("{}: {} {} {}", kUnknownOp, lhs.Type(), op, rhs.Type()));
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
    case ObjectType::kBuiltin: {
      const auto& fn_obj = obj.Cast<Builtin>();
      return fn_obj.func(args);
    }
    default:
      return ErrorObj(fmt::format("{}: {}", kNotAFunc, obj.Type()));
  }
}

Object Evaluator::EvalIfExpr(const IfExpr& expr, Environment& env) const {
  const auto cond = Evaluate(expr.cond, env);
  if (IsObjError(cond)) return cond;

  if (IsObjTruthy(cond)) {
    return EvalBlockStmt(expr.true_block, env);
  }

  if (!expr.false_block.empty()) {
    return EvalBlockStmt(expr.false_block, env);
  }

  return kNullObject;
}

}  // namespace monkey
