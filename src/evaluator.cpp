#include "monkey/evaluator.h"

#include <glog/logging.h>

namespace monkey {

Object Evaluate(const Program& node) {
  //  switch (node.Type()) {
  //    case NodeType::kProgram: {
  //      const auto* ptr = NodePtrCast<Program>(node.Ptr());
  //      return EvalStatements(ptr->statements);
  //    }
  //    case NodeType::kExprStmt: {
  //    }
  //    case NodeType::kIntLiteral: {
  //      IntObject obj;
  //      const auto* ptr = NodePtrCast<IntegerLiteral>(node.Ptr());
  //      obj.value = ptr->value;
  //      return obj;
  //    }

  //    default:
  //      return ObjectBase{};
  //  }
  return EvalStatements(node.statements);
}

Object EvalStatements(const std::vector<Statement>& stmts) {
  Object obj{ObjectBase{}};
  for (const auto& stmt : stmts) {
    obj = Evaluate(stmt);
  }
  return obj;
}

Object Evaluate(const Statement& stmt) {
  switch (stmt.Type()) {
    case NodeType::kExprStmt:
      return Evaluate(stmt.Expr());
    default:
      return ObjectBase{};
  }
}

Object Evaluate(const Expression& expr) {
  switch (expr.Type()) {
    case NodeType::kIntLiteral: {
      const auto* ptr = static_cast<IntegerLiteral*>(expr.Ptr());
      IntObject obj;
      obj.value = ptr->value;
      return obj;
    }
    default:
      return ObjectBase{};
  }
}

}  // namespace monkey
