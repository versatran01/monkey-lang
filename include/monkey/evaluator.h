#pragma once

#include "monkey/ast.h"
#include "monkey/object.h"

namespace monkey {

Object Evaluate(const Program& node);
Object Evaluate(const Statement& stmt);
Object Evaluate(const Expression& expr);

Object EvalStatements(const std::vector<Statement>& stmts);

}  // namespace monkey
