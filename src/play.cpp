#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/parser.h"

using namespace monkey;

int main() {
  const std::string input = R"raw(
    if (1 == 1) {
      let x = 2;
      return x;
    } else {
      let x = 3;
      return x;
    }
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();
  LOG(INFO) << program.NumStatments();

  for (const auto& stmt : program.statements) {
    LOG(INFO) << stmt.String();
  }
  LOG(INFO) << "\n" << program.String();
  LOG(INFO) << fmt::format("{}", parser.ErrorMsg());
}
