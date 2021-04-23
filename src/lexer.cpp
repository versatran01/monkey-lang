#include "monkey/lexer.h"

namespace monkey {

namespace {

bool IsDigit(char c) { return std::isdigit(c); }
bool IsLetter(char c) { return std::isalpha(c) || c == '_'; }

}  // namespace

Lexer::Lexer(std::string input) : input_(std::move(input)) { ReadChar(); }

Token Lexer::NextToken() {
  Token token;

  SkipWhitespace();

  switch (ch_) {
    case '=':
      if (PeekChar() == '=') {
        auto prev_ch = ch_;
        ReadChar();
        token = Token{token_type::kEq, std::string{prev_ch} + ch_};
      } else {
        token = Token{token_type::kAssign, {ch_}};
      }
      break;
    case ';':
      token = Token{token_type::kSemicolon, {ch_}};
      break;
    case '(':
      token = Token{token_type::kLParen, {ch_}};
      break;
    case ')':
      token = Token{token_type::kRParen, {ch_}};
      break;
    case ',':
      token = Token{token_type::kComma, {ch_}};
      break;
    case '!':
      if (PeekChar() == '=') {
        auto prev_ch = ch_;
        ReadChar();
        token = Token{token_type::kNe, std::string{prev_ch} + ch_};
      } else {
        token = Token{token_type::kBang, {ch_}};
      }
      break;
    case '+':
      token = Token{token_type::kPlus, {ch_}};
      break;
    case '-':
      token = Token{token_type::kMinus, {ch_}};
      break;
    case '*':
      token = Token{token_type::kAsterisk, {ch_}};
      break;
    case '/':
      token = Token{token_type::kSlash, {ch_}};
      break;
    case '<':
      token = Token{token_type::kLt, {ch_}};
      break;
    case '>':
      token = Token{token_type::kGt, {ch_}};
      break;
    case '{':
      token = Token{token_type::kLBrace, {ch_}};
      break;
    case '}':
      token = Token{token_type::kRBrace, {ch_}};
      break;
    case 0:
      token = Token{token_type::kEof, ""};
      break;
    default:
      if (IsLetter(ch_)) {
        token.literal = ReadIdentifier();
        token.type = LookupIdentifier(token.literal);
        return token;  // early return to prevent extra ReadChar();
      } else if (IsDigit(ch_)) {
        token.type = token_type::kInt;
        token.literal = ReadNumber();
        return token;  // early return to prevent extra ReadChar();

      } else {
        token = Token{token_type::kIllegal, {ch_}};
      }
  }

  ReadChar();
  return token;
}

void Lexer::ReadChar() {
  if (position_ >= static_cast<int>(input_.size())) {
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

char Lexer::PeekChar() const {
  return (read_position_ >= static_cast<int>(input_.size()))
             ? 0
             : input_[read_position_];
}

std::string Lexer::ReadNumber() {
  const int start = position_;
  while (IsDigit(ch_)) {
    ReadChar();
  }
  return input_.substr(start, position_ - start);
}

std::string Lexer::ReadIdentifier() {
  const int start = position_;
  while (IsLetter(ch_)) {
    ReadChar();
  }
  return input_.substr(start, position_ - start);
}

}  // namespace monkey
