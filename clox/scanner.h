#pragma once

#include "common.h"

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
  X(FOR)                                                                       \
  X(FUN)                                                                       \
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
  X(ERROR)                                                                     \
  X(EOF)

#define X(token) TOKEN_##token,
typedef enum { TOKEN_TYPES } TokenType;
#undef X

typedef struct {
  const char *start;
  size_t length;
  // Arranged this way for better struct packing.
  TokenType type;
  unsigned line;
} Token;

void initScanner(const char *source);
Token scanToken(void);
const char *getTokenTypeName(TokenType type);
