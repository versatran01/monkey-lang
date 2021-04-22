#include "monkey/lexer.h"

#include <gtest/gtest.h>

namespace {

using monkey::Lexer;

TEST(LexerTest, TestNextToken) {
  const std::string input = "=+(){},;";

  Lexer lexer(input);
}

}  // namespace