#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/parser.h"

using namespace monkey;

int main() {
  const std::string input = R"raw(
    let x = 5;
    let y = 10;
    let z = 123;
   )raw";

  Parser parser{input};

  auto program = parser.ParseProgram();
  LOG(INFO) << program.statements.size();

  for (const auto& stmt : program.statements) {
    LOG(INFO) << stmt.TokenLiteral();
  }
  LOG(INFO) << "\n" << program.String();
  LOG(INFO) << fmt::format("{}", parser.errors());
}
