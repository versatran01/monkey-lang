#include "monkey/lexer.h"

#include <cctype>

namespace monkey {

namespace {

bool IsDigit(char c) { return std::isdigit(c); }
bool IsLetter(char c) { return std::isalpha(c) || c == '_'; }

}  // namespace

Lexer::Lexer(std::string input) : input_{std::move(input)} { ReadChar(); }

Token Lexer::ReadDualToken(TokenType type1,
                           char next_ch,
                           TokenType type2) noexcept {
  if (PeekChar() == next_ch) {
    auto prev_ch = ch_;
    ReadChar();
    return {type2, std::string{prev_ch} + ch_};
  }

  return {type1, {ch_}};
}

Token Lexer::NextToken() {
  Token token;

  SkipWhitespace();

  switch (ch_) {
    case '=':
      token = ReadDualToken(TokenType::kAssign, '=', TokenType::kEq);
      break;
    case '!':
      token = ReadDualToken(TokenType::kBang, '=', TokenType::kNe);
      break;
    case '<':
      token = ReadDualToken(TokenType::kLt, '=', TokenType::kLe);
      break;
    case '>':
      token = ReadDualToken(TokenType::kGt, '=', TokenType::kGe);
      break;
    case ';':
      token = {TokenType::kSemicolon, {ch_}};
      break;
    case ':':
      token = {TokenType::kColon, {ch_}};
      break;
    case '(':
      token = {TokenType::kLParen, {ch_}};
      break;
    case ')':
      token = {TokenType::kRParen, {ch_}};
      break;
    case ',':
      token = {TokenType::kComma, {ch_}};
      break;
    case '+':
      token = {TokenType::kPlus, {ch_}};
      break;
    case '-':
      token = {TokenType::kMinus, {ch_}};
      break;
    case '*':
      token = {TokenType::kAsterisk, {ch_}};
      break;
    case '/':
      token = {TokenType::kSlash, {ch_}};
      break;
    case '{':
      token = {TokenType::kLBrace, {ch_}};
      break;
    case '}':
      token = {TokenType::kRBrace, {ch_}};
      break;
    case '[':
      token = {TokenType::kLBracket, {ch_}};
      break;
    case ']':
      token = {TokenType::kRBracket, {ch_}};
      break;
    case '"':
      token = {TokenType::kStr, ReadString()};
      break;
    case 0:
      token = {TokenType::kEof, ""};
      break;
    default:
      if (IsLetter(ch_)) {
        token.literal = ReadIdentifier();
        token.type = GetKeywordType(token.literal);
        return token;  // early return to prevent extra ReadChar();
      } else if (IsDigit(ch_)) {
        token.type = TokenType::kInt;
        token.literal = ReadNumber();
        return token;  // early return to prevent extra ReadChar();

      } else {
        token = {TokenType::kIllegal, {ch_}};
      }
  }

  ReadChar();
  return token;
}

void Lexer::ReadChar() noexcept {
  if (position_ >= input_.size()) {
    ch_ = 0;
  } else {
    ch_ = input_[read_position_];
  }
  position_ = read_position_;
  ++read_position_;
}

void Lexer::SkipWhitespace() {
  while (std::isspace(ch_)) {
    ReadChar();
  }
}

char Lexer::PeekChar() const noexcept {
  return (read_position_ >= input_.size()) ? char{0} : input_[read_position_];
}

std::string Lexer::ReadNumber() {
  const auto start = position_;
  while (IsDigit(ch_)) {
    ReadChar();
  }
  return input_.substr(start, position_ - start);
}

std::string Lexer::ReadIdentifier() {
  const auto start = position_;
  while (IsLetter(ch_)) {
    ReadChar();
  }
  return input_.substr(start, position_ - start);
}

std::string Lexer::ReadString() {
  const auto start = position_ + 1;
  while (true) {
    ReadChar();
    if (ch_ == '"' || ch_ == 0) {
      break;
    }
  }
  return input_.substr(start, position_ - start);
}

}  // namespace monkey
