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

TEST(LexerTest, TestSimpleCode) {
  const std::string input = R"foo(
    let five = 5;
    let ten = 10;
    let add = fn(x, y) {
    x + y;
    };
    let result = add(five, ten);
  )foo";

  Lexer lexer(input);
  std::vector<Token> true_tokens{
      {token_type::kLet, "let"},     {token_type::kIdent, "five"},
      {token_type::kAssign, "="},    {token_type::kInt, "5"},
      {token_type::kSemicolon, ";"}, {token_type::kLet, "let"},
      {token_type::kIdent, "ten"},   {token_type::kAssign, "="},
      {token_type::kInt, "10"},      {token_type::kSemicolon, ";"},
      {token_type::kLet, "let"},     {token_type::kIdent, "add"},
      {token_type::kAssign, "="},    {token_type::kFunction, "fn"},
      {token_type::kLParen, "("},    {token_type::kIdent, "x"},
      {token_type::kComma, ","},     {token_type::kIdent, "y"},
      {token_type::kRParen, ")"},    {token_type::kLBrace, "{"},
      {token_type::kIdent, "x"},     {token_type::kPlus, "+"},
      {token_type::kIdent, "y"},     {token_type::kSemicolon, ";"},
      {token_type::kLBrace, "}"},    {token_type::kSemicolon, ";"},
      {token_type::kLet, "let"},     {token_type::kIdent, "result"},
      {token_type::kAssign, "="},    {token_type::kIdent, "add"},
      {token_type::kLParen, "("},    {token_type::kIdent, "five"},
      {token_type::kComma, ","},     {token_type::kIdent, "ten"},
      {token_type::kRParen, ")"},    {token_type::kSemicolon, ";"},
      {token_type::kEof, ""}};

  for (const auto& true_token : true_tokens) {
    const auto next_token = lexer.NextToken();
    EXPECT_EQ(next_token.type, true_token.type);
    EXPECT_EQ(next_token.literal, true_token.literal);
  }
}

}  // namespace

}  // namespace monkey
