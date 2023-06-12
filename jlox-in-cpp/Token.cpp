#include "Token.h"

#define STR_(s) #s
#define STR(s) STR_(s)
#define X(token) STR(token),
static const char *tokenTypeNames[] = {TOKEN_TYPES};
#undef X

std::ostream &operator<<(std::ostream &o, TokenType tokenType) {
  o << tokenTypeNames[static_cast<int>(tokenType)];
  return o;
}

std::ostream &operator<<(std::ostream &o, const Token &token) {
  o << token.type << " " << token.lexeme;
  if (const std::string_view *literal =
          std::get_if<std::string_view>(&token.literal)) {
    o << " " << *literal;
  } else if (const double *literal = std::get_if<double>(&token.literal)) {
    o << " " << *literal;
  }
  return o;
}
