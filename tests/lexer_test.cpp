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
    ASSERT_EQ(next_token.type, true_token.type);
    ASSERT_EQ(next_token.literal, true_token.literal);
  }
}

TEST(LexerTest, TestSimpleCode) {
  const std::string input = R"raw(
    let five = 5;
    let ten = 10;
    let add = fn(x, y) {
    x + y;
    };
    let result = add(five, ten);
    !-/*5;
    5 < 10 > 5;
    return true;
    return false;
    10 == 10;
    10 != 9;
  )raw";

  Lexer lexer(input);
  std::vector<Token> true_tokens{
      {token_type::kLet, "let"},       {token_type::kIdent, "five"},
      {token_type::kAssign, "="},      {token_type::kInt, "5"},
      {token_type::kSemicolon, ";"},   {token_type::kLet, "let"},
      {token_type::kIdent, "ten"},     {token_type::kAssign, "="},
      {token_type::kInt, "10"},        {token_type::kSemicolon, ";"},
      {token_type::kLet, "let"},       {token_type::kIdent, "add"},
      {token_type::kAssign, "="},      {token_type::kFunction, "fn"},
      {token_type::kLParen, "("},      {token_type::kIdent, "x"},
      {token_type::kComma, ","},       {token_type::kIdent, "y"},
      {token_type::kRParen, ")"},      {token_type::kLBrace, "{"},
      {token_type::kIdent, "x"},       {token_type::kPlus, "+"},
      {token_type::kIdent, "y"},       {token_type::kSemicolon, ";"},
      {token_type::kRBrace, "}"},      {token_type::kSemicolon, ";"},
      {token_type::kLet, "let"},       {token_type::kIdent, "result"},
      {token_type::kAssign, "="},      {token_type::kIdent, "add"},
      {token_type::kLParen, "("},      {token_type::kIdent, "five"},
      {token_type::kComma, ","},       {token_type::kIdent, "ten"},
      {token_type::kRParen, ")"},      {token_type::kSemicolon, ";"},
      {token_type::kBang, "!"},        {token_type::kMinus, "-"},
      {token_type::kSlash, "/"},       {token_type::kAsterisk, "*"},
      {token_type::kInt, "5"},         {token_type::kSemicolon, ";"},
      {token_type::kInt, "5"},         {token_type::kLt, "<"},
      {token_type::kInt, "10"},        {token_type::kGt, ">"},
      {token_type::kInt, "5"},         {token_type::kSemicolon, ";"},
      {token_type::kReturn, "return"}, {token_type::kTrue, "true"},
      {token_type::kSemicolon, ";"},   {token_type::kReturn, "return"},
      {token_type::kFalse, "false"},   {token_type::kSemicolon, ";"},
      {token_type::kInt, "10"},        {token_type::kEq, "=="},
      {token_type::kInt, "10"},        {token_type::kSemicolon, ";"},
      {token_type::kInt, "10"},        {token_type::kNe, "!="},
      {token_type::kInt, "9"},         {token_type::kSemicolon, ";"},
      {token_type::kEof, ""}};

  for (const auto& true_token : true_tokens) {
    const auto next_token = lexer.NextToken();
    ASSERT_EQ(next_token.type, true_token.type);
    ASSERT_EQ(next_token.literal, true_token.literal);
  }
}

}  // namespace

}  // namespace monkey
