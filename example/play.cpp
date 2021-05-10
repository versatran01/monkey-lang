#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/evaluator.h"
#include "monkey/function.h"
#include "monkey/object.h"
#include "monkey/parser.h"

using namespace monkey;

int main() {
  const std::string input = "fn(x) { x + 2; return 3; };";
  Parser parser{input};
  const auto program = parser.ParseProgram();
  Evaluator eval;
  Environment env;
  const auto obj = eval.Evaluate(program, env);
  LOG(INFO) << "=== BEFORE";
  LOG(INFO) << env;
  LOG(INFO) << obj.Inspect();

  env.Set("add", obj);
  LOG(INFO) << "=== SET";
  LOG(INFO) << env;
  LOG(INFO) << obj.Inspect();

  auto obj2 = *env.Get("add");
  LOG(INFO) << "=== GET";
  LOG(INFO) << env;
  LOG(INFO) << obj2.Inspect();
}
