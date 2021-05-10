#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>

#include "monkey/evaluator.h"
#include "monkey/parser.h"

namespace monkey {

const std::string kPrompt = ">> ";

void StartRepl() {
  std::string line;
  Evaluator eval;
  Environment env;

  while (true) {
    fmt::print(kPrompt);
    std::getline(std::cin, line);

    Parser parser{line};
    auto program = parser.ParseProgram();

    if (!program.Ok()) {
      fmt::print("{}\n", parser.ErrorMsg());
    }

    const auto result = eval.Evaluate(program, env);
    if (result.Ok()) {
      fmt::print("{}\n", result.Inspect());
    }
  }
}

}  // namespace monkey

int main() {
  fmt::print("Hello!, This is the Monkey programming language\n");
  monkey::StartRepl();
}
