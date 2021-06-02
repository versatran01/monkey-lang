#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/environment.h"
#include "monkey/evaluator.h"
#include "monkey/parser.h"

using namespace monkey;
using namespace std::string_literals;

int main() {
  Evaluator eval;
  Environment env;
  std::string fibcode = R"r(  let fibonacci = fn(x) {
        if (x == 0) {
            return 0;
        } else {
            if (x == 1) {
                return 1;
            } else {
                return fibonacci(x - 1) + fibonacci(x - 2);
            }
        }
    };)r";

  std::string input = fibcode + "fibonacci(2);";

  Parser parser{input};
  auto program = parser.ParseProgram();

  if (!program.Ok()) {
    fmt::print("{}\n", parser.ErrorMsg());
    return 0;
  }

  auto obj = eval.Evaluate(program, env);
  LOG(INFO) << env;
  fmt::print("{}\n", obj);
}
