#include <fmt/core.h>

#include "monkey/repl.h"

int main() {
  fmt::print("Hello!, This is the Monkey programming language\n");
  monkey::StartRepl();
}
