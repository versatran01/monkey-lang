#include "monkey/lexer.h"

namespace monkey {

namespace {

bool IsLetter(char c) { return std::isalpha(c) || c == '_'; }

}  // namespace

Lexer::Lexer(std::string input) : input_(std::move(input)) { ReadChar(); }

Token Lexer::NextToken() {
  Token token;

  switch (ch_) {
    case '=':
      token = Token{token_type::kAssign, {ch_}};
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
    case '+':
      token = Token{token_type::kPlus, {ch_}};
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

std::string Lexer::ReadIdentifier() {
  const int start = position_;
  while (IsLetter(ch_)) {
    ReadChar();
  }
  return input_.substr(start, position_ - start);
}

}  // namespace monkey
