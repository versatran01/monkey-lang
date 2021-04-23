#include <fmt/format.h>

#include <iostream>

#include "monkey/lexer.h"

namespace monkey {

const std::string kPrompt = ">>";

void StartRepl() {
  std::string line;

  while (true) {
    fmt::print(kPrompt);
    std::getline(std::cin, line);

    Lexer lexer{line};

    for (Token t = lexer.NextToken(); t.type != token_type::kEof;
         t = lexer.NextToken()) {
      fmt::print("{},\t {}\n", t.type, t.literal);
    }
  }
}

}  // namespace monkey
