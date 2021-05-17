#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include <iostream>

#include "monkey/compiler.h"
#include "monkey/evaluator.h"
#include "monkey/parser.h"
#include "monkey/vm.h"

ABSL_FLAG(bool, eval, true, "Run evaluator.");

namespace monkey {

const std::string kPrompt = ">> ";

void StartReplComp() {
  std::string line;
  Compiler comp;

  while (true) {
    fmt::print(kPrompt);
    std::getline(std::cin, line);

    Parser parser{line};
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

    VirtualMachine vm;
    const auto status = vm.Run(*bc);
    if (!status.ok()) {
      fmt::print("Executing bytecode failed:\n{}\n", status);
      continue;
    }

    fmt::print("{}\n", vm.Top().Inspect());
  }
}

void StartReplEval() {
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
      continue;
    }

    const auto obj = eval.Evaluate(program, env);
    if (obj.Ok()) {
      fmt::print("{}\n", obj.Inspect());
    }
  }
}

}  // namespace monkey

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  fmt::print("Hello! This is the Monkey programming language\n");

  if (absl::GetFlag(FLAGS_eval)) {
    fmt::print("Running Evaluator\n");
    monkey::StartReplEval();
  } else {
    fmt::print("Running Compiler\n");
    monkey::StartReplComp();
  }
}
