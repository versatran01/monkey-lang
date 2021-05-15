#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>

#include "monkey/parser.h"

namespace monkey {

const std::string kPrompt = ">> ";

void StartRppl() {
  std::string line;

  while (true) {
    fmt::print(kPrompt);
    std::getline(std::cin, line);

    Parser parser{line};
    auto program = parser.ParseProgram();

    if (!program.Ok()) {
      fmt::print("{}\n", parser.ErrorMsg());
    }

    fmt::print("{}\n", program.String());
  }
}

}  // namespace monkey

int main() {
  fmt::print("Hello!, This is the Monkey programming language\n");
  monkey::StartRppl();
}
