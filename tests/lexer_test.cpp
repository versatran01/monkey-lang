#include "monkey/lexer.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace {
using namespace monkey;

TEST(LexerTest, TestTokenTypeOstream) {
  auto tt = TokenType::kIllegal;
  LOG(INFO) << tt;
}

TEST(LexerTest, TestTokenOstream) {
  auto t = Token{TokenType::kAssign, "="};
  LOG(INFO) << t;
}

TEST(LexerTest, TestNextToken) {
  const std::string input = "=+(){},;";

  Lexer lexer(input);
  std::vector<Token> true_tokens{{TokenType::kAssign, "="},
                                 {TokenType::kPlus, "+"},
                                 {TokenType::kLParen, "("},
                                 {TokenType::kRParen, ")"},
                                 {TokenType::kLBrace, "{"},
                                 {TokenType::kRBrace, "}"},
                                 {TokenType::kComma, ","},
                                 {TokenType::kSemicolon, ";"},
                                 {TokenType::kEof, ""}};

  for (const auto& true_token : true_tokens) {
    SCOPED_TRACE(true_token);
    const auto next_token = lexer.NextToken();
    ASSERT_EQ(next_token.type, true_token.type);
    EXPECT_EQ(next_token.literal, true_token.literal);
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
    "foobar"
    "foo bar"
    [1, 2];
    {"foo": "bar"}
  )raw";

  Lexer lexer(input);
  std::vector<Token> true_tokens{
      {TokenType::kLet, "let"},       {TokenType::kIdent, "five"},
      {TokenType::kAssign, "="},      {TokenType::kInt, "5"},
      {TokenType::kSemicolon, ";"},   {TokenType::kLet, "let"},
      {TokenType::kIdent, "ten"},     {TokenType::kAssign, "="},
      {TokenType::kInt, "10"},        {TokenType::kSemicolon, ";"},
      {TokenType::kLet, "let"},       {TokenType::kIdent, "add"},
      {TokenType::kAssign, "="},      {TokenType::kFunc, "fn"},
      {TokenType::kLParen, "("},      {TokenType::kIdent, "x"},
      {TokenType::kComma, ","},       {TokenType::kIdent, "y"},
      {TokenType::kRParen, ")"},      {TokenType::kLBrace, "{"},
      {TokenType::kIdent, "x"},       {TokenType::kPlus, "+"},
      {TokenType::kIdent, "y"},       {TokenType::kSemicolon, ";"},
      {TokenType::kRBrace, "}"},      {TokenType::kSemicolon, ";"},
      {TokenType::kLet, "let"},       {TokenType::kIdent, "result"},
      {TokenType::kAssign, "="},      {TokenType::kIdent, "add"},
      {TokenType::kLParen, "("},      {TokenType::kIdent, "five"},
      {TokenType::kComma, ","},       {TokenType::kIdent, "ten"},
      {TokenType::kRParen, ")"},      {TokenType::kSemicolon, ";"},
      {TokenType::kBang, "!"},        {TokenType::kMinus, "-"},
      {TokenType::kSlash, "/"},       {TokenType::kAsterisk, "*"},
      {TokenType::kInt, "5"},         {TokenType::kSemicolon, ";"},
      {TokenType::kInt, "5"},         {TokenType::kLt, "<"},
      {TokenType::kInt, "10"},        {TokenType::kGt, ">"},
      {TokenType::kInt, "5"},         {TokenType::kSemicolon, ";"},
      {TokenType::kReturn, "return"}, {TokenType::kTrue, "true"},
      {TokenType::kSemicolon, ";"},   {TokenType::kReturn, "return"},
      {TokenType::kFalse, "false"},   {TokenType::kSemicolon, ";"},
      {TokenType::kInt, "10"},        {TokenType::kEq, "=="},
      {TokenType::kInt, "10"},        {TokenType::kSemicolon, ";"},
      {TokenType::kInt, "10"},        {TokenType::kNe, "!="},
      {TokenType::kInt, "9"},         {TokenType::kSemicolon, ";"},
      {TokenType::kStr, "foobar"},    {TokenType::kStr, "foo bar"},
      {TokenType::kLBracket, "["},    {TokenType::kInt, "1"},
      {TokenType::kComma, ","},       {TokenType::kInt, "2"},
      {TokenType::kRBracket, "]"},    {TokenType::kSemicolon, ";"},
      {TokenType::kLBrace, "{"},      {TokenType::kStr, "foo"},
      {TokenType::kColon, ":"},       {TokenType::kStr, "bar"},
      {TokenType::kRBrace, "}"},      {TokenType::kEof, ""},
  };

  for (const auto& true_token : true_tokens) {
    SCOPED_TRACE(true_token);
    const auto next_token = lexer.NextToken();
    ASSERT_EQ(next_token.type, true_token.type);
    EXPECT_EQ(next_token.literal, true_token.literal);
  }
}

}  // namespace
