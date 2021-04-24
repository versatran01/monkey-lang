#include "monkey/parser.h"

namespace monkey {

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
  // Read two tokens, so curToken and peekToken are both set
  NextToken();
  NextToken();
}

void Parser::NextToken() {
  curr_token_ = peek_token_;
  peek_token_ = lexer_.NextToken();
}

}  // namespace monkey
