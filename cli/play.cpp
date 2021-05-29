#include <fmt/ranges.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "monkey/compiler.h"
#include "monkey/parser.h"
#include "monkey/vm.h"

using namespace monkey;
using namespace std::string_literals;

int main() {
  Compiler comp;
  VirtualMachine vm;

  const auto line1 = "let a = 1;"s;
  const auto line2 = "let b = 2;"s;

  std::vector<std::string> lines = {"let a = 1;", "let b = 2;"};

  for (const auto& line : lines) {
    Parser parser{line1};
    auto program = parser.ParseProgram();

    if (!program.Ok()) {
      fmt::print("{}\n", parser.ErrorMsg());
      continue;
    }

    const auto bc = comp.Compile(program);
    if (!bc.ok()) {
      fmt::print("Compilation failed:\n{}\n", bc.status());
      continue;
    }

    const auto status = vm.Run(*bc);
    if (!status.ok()) {
      fmt::print("Executing bytecode failed:\n{}\n", status);
      continue;
    }
  }
}
