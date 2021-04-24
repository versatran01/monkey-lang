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
        token = Token{TokenType::kEq, std::string{prev_ch} + ch_};
      } else {
        token = Token{TokenType::kAssign, {ch_}};
      }
      break;
    case ';':
      token = Token{TokenType::kSemicolon, {ch_}};
      break;
    case '(':
      token = Token{TokenType::kLParen, {ch_}};
      break;
    case ')':
      token = Token{TokenType::kRParen, {ch_}};
      break;
    case ',':
      token = Token{TokenType::kComma, {ch_}};
      break;
    case '!':
      if (PeekChar() == '=') {
        auto prev_ch = ch_;
        ReadChar();
        token = Token{TokenType::kNe, std::string{prev_ch} + ch_};
      } else {
        token = Token{TokenType::kBang, {ch_}};
      }
      break;
    case '+':
      token = Token{TokenType::kPlus, {ch_}};
      break;
    case '-':
      token = Token{TokenType::kMinus, {ch_}};
      break;
    case '*':
      token = Token{TokenType::kAsterisk, {ch_}};
      break;
    case '/':
      token = Token{TokenType::kSlash, {ch_}};
      break;
    case '<':
      token = Token{TokenType::kLt, {ch_}};
      break;
    case '>':
      token = Token{TokenType::kGt, {ch_}};
      break;
    case '{':
      token = Token{TokenType::kLBrace, {ch_}};
      break;
    case '}':
      token = Token{TokenType::kRBrace, {ch_}};
      break;
    case 0:
      token = Token{TokenType::kEof, ""};
      break;
    default:
      if (IsLetter(ch_)) {
        token.literal = ReadIdentifier();
        token.type = LookupIdentifier(token.literal);
        return token;  // early return to prevent extra ReadChar();
      } else if (IsDigit(ch_)) {
        token.type = TokenType::kInt;
        token.literal = ReadNumber();
        return token;  // early return to prevent extra ReadChar();

      } else {
        token = Token{TokenType::kIllegal, {ch_}};
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
