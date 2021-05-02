#include <fmt/ranges.h>
#include <glog/logging.h>

#include "monkey/parser.h"

using namespace monkey;

int main() {
  const std::string input = R"raw(
    5 == 5;
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
