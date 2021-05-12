#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/evaluator.h"
#include "monkey/object.h"
#include "monkey/parser.h"

using namespace monkey;

int main() {
  const std::string input = R"raw(
    let newAdder = fn(x) {
        fn(y) { x + y };
    };
    let addTwo = newAdder(2);
    addTwo(2);)raw";

  Parser parser{input};
  const auto program = parser.ParseProgram();
  Evaluator eval;
  Environment env;
  const auto obj = eval.Evaluate(program, env);
  LOG(INFO) << obj.Inspect();
}
