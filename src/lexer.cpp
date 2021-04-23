#include "monkey/lexer.h"

namespace monkey {

Lexer::Lexer(std::string input) : input_(std::move(input)) { ReadChar(); }

Token Lexer::NextToken() noexcept {
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
      token.type = token_type::kIllegal;
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

}  // namespace monkey
