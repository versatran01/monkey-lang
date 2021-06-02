#include <benchmark/benchmark.h>
#include <fmt/core.h>

#include "monkey/compiler.h"
#include "monkey/evaluator.h"
#include "monkey/parser.h"
#include "monkey/vm.h"

namespace {
using namespace monkey;

const std::string kFibonacciCode = R"r(
    let fibonacci = fn(x) {
        if (x == 0) {
            return 0;
        } else {
            if (x == 1) {
                return 1;
            } else {
                fibonacci(x - 1) + fibonacci(x - 2);
            }
        }
    };
    )r";

std::string MakeFibonacciCall(int n) {
  return kFibonacciCode + fmt::format("fibonacci({});", n);
}

void BM_Evaluator(benchmark::State& state) {
  Evaluator eval;
  Environment env;
  Parser parser{MakeFibonacciCall(state.range(0))};

  auto program = parser.ParseProgram();
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(eval.Evaluate(program, env));
  }
}
BENCHMARK(BM_Evaluator)->RangeMultiplier(2)->Range(1, 8);


void BM_Compiler(benchmark::State& state) {
  Compiler comp;
  VirtualMachine vm;
  Parser parser{MakeFibonacciCall(state.range(0))};
  auto program = parser.ParseProgram();

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize([&]() {
      const auto bc = comp.Compile(program);
      const auto status = vm.Run(*bc);
      return vm.Last();
    }());
  }
}
BENCHMARK(BM_Compiler)->RangeMultiplier(2)->Range(1, 8);

}  // namespace
