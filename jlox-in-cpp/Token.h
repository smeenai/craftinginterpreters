#pragma once

#include <ostream>
#include <string_view>
#include <variant>

// This is kinda gross, but it lets us print enum values nicely
// https://en.wikipedia.org/wiki/X_Macro
#define TOKEN_TYPES                                                            \
  /* Single-character tokens. */                                               \
  X(LEFT_PAREN)                                                                \
  X(RIGHT_PAREN)                                                               \
  X(LEFT_BRACE)                                                                \
  X(RIGHT_BRACE)                                                               \
  X(COMMA)                                                                     \
  X(DOT)                                                                       \
  X(MINUS)                                                                     \
  X(PLUS)                                                                      \
  X(SEMICOLON)                                                                 \
  X(SLASH)                                                                     \
  X(STAR)                                                                      \
  /* One or two character tokens. */                                           \
  X(BANG)                                                                      \
  X(BANG_EQUAL)                                                                \
  X(EQUAL)                                                                     \
  X(EQUAL_EQUAL)                                                               \
  X(GREATER)                                                                   \
  X(GREATER_EQUAL)                                                             \
  X(LESS)                                                                      \
  X(LESS_EQUAL)                                                                \
  /* Literals. */                                                              \
  X(IDENTIFIER)                                                                \
  X(STRING)                                                                    \
  X(NUMBER)                                                                    \
  /* Keywords. */                                                              \
  X(AND)                                                                       \
  X(CLASS)                                                                     \
  X(ELSE)                                                                      \
  X(FALSE)                                                                     \
  X(FUN)                                                                       \
  X(FOR)                                                                       \
  X(IF)                                                                        \
  X(NIL)                                                                       \
  X(OR)                                                                        \
  X(PRINT)                                                                     \
  X(RETURN)                                                                    \
  X(SUPER)                                                                     \
  X(THIS)                                                                      \
  X(TRUE)                                                                      \
  X(VAR)                                                                       \
  X(WHILE)                                                                     \
  /* Other. */                                                                 \
  X(TOKEN_EOF)

#define X(token) token,
enum class TokenType { TOKEN_TYPES };
#undef X

std::ostream &operator<<(std::ostream &o, TokenType tokenType);

class Token {
  const TokenType type;
#pragma clang diagnostic push
  // Temporary until we have a use
#pragma clang diagnostic ignored "-Wunused-private-field"
  const unsigned line; // ordered this way for better packing
#pragma clang diagnostic pop
  const std::string_view lexeme;
  const std::variant<std::monostate, std::string_view, double> literal;

public:
  Token(TokenType type, std::string_view lexeme, int line)
      : type(type), line(line), lexeme(lexeme) {}

  Token(TokenType type, std::string_view lexeme, std::string_view literal,
        int line)
      : type(type), line(line), lexeme(lexeme), literal(literal) {}

  Token(TokenType type, std::string_view lexeme, double literal, int line)
      : type(type), line(line), lexeme(lexeme), literal(literal) {}

  friend std::ostream &operator<<(std::ostream &o, const Token &token);
};
