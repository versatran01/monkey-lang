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
  return ObjectBase{};
}

Object Evaluate(const Statement& stmt) {}

}  // namespace monkey
