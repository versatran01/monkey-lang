#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/evaluator.h"
#include "monkey/object.h"
#include "monkey/parser.h"

using namespace monkey;

int main() {
  const std::string input = "fn(x) { x + 2; return 3; };";
  Parser parser{input};
  const auto program = parser.ParseProgram();
  LOG(INFO) << program.statements[0].String();
  LOG(INFO) << program.statements[0].Expr().String();

  const StmtNode& stmt = program.statements[0];
  LOG(INFO) << "new: " << stmt.String();
  LOG(INFO) << "old: " << program.statements[0].String();
}
