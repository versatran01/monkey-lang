#include "monkey/lexer.h"

#include <gtest/gtest.h>

namespace monkey {

namespace {

TEST(LexerTest, TestNextToken) {
  const std::string input = "=+(){},;";

  Lexer lexer(input);
  std::vector<Token> true_tokens{
      {token_type::kAssign, "="}, {token_type::kPlus, "+"},
      {token_type::kLParen, "("}, {token_type::kRParen, ")"},
      {token_type::kLBrace, "{"}, {token_type::kRBrace, "}"},
      {token_type::kComma, ","},  {token_type::kSemicolon, ";"},
      {token_type::kEof, ""}};

  for (const auto& true_token : true_tokens) {
    const auto next_token = lexer.NextToken();
    EXPECT_EQ(next_token.type, true_token.type);
    EXPECT_EQ(next_token.literal, true_token.literal);
  }
}

}  // namespace

}  // namespace monkey
