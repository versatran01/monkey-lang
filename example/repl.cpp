#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>

#include "monkey/parser.h"

namespace monkey {

const std::string kPrompt = ">> ";

void StartRepl() {}

}  // namespace monkey

int main() {
  fmt::print("Hello!, This is the Monkey programming language\n");
  monkey::StartRepl();
}
