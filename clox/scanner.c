#include "scanner.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

typedef struct {
  const char *start;
  const char *current;
  unsigned line;
} Scanner;

static Scanner scanner;

void initScanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static bool isAlpha(char c) {
  // Not using isalpha to avoid any locale issues.
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isAtEnd() { return *scanner.current == '\0'; }

static char peek() { return *scanner.current; }

static char peekNext() { return isAtEnd() ? '\0' : scanner.current[1]; }

static char advance() { return *scanner.current++; }

static bool match(char expected) {
  if (isAtEnd())
    return false;
  if (peek() != expected)
    return false;
  // I just wanted to use the comma operator outside a for loop for once :p
  return advance(), true;
}

static Token makeToken(TokenType type) {
  Token token = {
      .type = type,
      .start = scanner.start,
      .length = scanner.current - scanner.start,
      .line = scanner.line,
  };
  return token;
}

static Token errorToken(const char *message) {
  Token token = {
      .type = TOKEN_ERROR,
      .start = message,
      .length = strlen(message),
      .line = scanner.line,
  };
  return token;
}

static void skipWhitespace() {
  while (true) {
    char c = peek();
    switch (c) {
    case '\n':
      ++scanner.line;
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;

    case '/':
      if (peekNext() == '/') {
        // A comment goes until the end of the line.
        while (peek() != '\n' && !isAtEnd())
          advance();
      } else {
        return;
      }

    default:
      return;
    }
  }
}

static TokenType checkKeyword(size_t start, size_t length, const char *rest,
                              TokenType type) {
  if ((size_t)(scanner.current - scanner.start) == start + length &&
      memcmp(scanner.start + start, rest, length) == 0)
    return type;
  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (*scanner.start) {
  case 'a':
    return checkKeyword(1, 2, "nd", TOKEN_AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TOKEN_CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TOKEN_ELSE);

  case 'f':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TOKEN_FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TOKEN_FUN);
      }
    }
    break;

  case 'i':
    return checkKeyword(1, 1, "f", TOKEN_IF);
  case 'n':
    return checkKeyword(1, 2, "il", TOKEN_NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TOKEN_OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TOKEN_PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TOKEN_SUPER);

  case 't':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'h':
        return checkKeyword(2, 2, "is", TOKEN_THIS);
      case 'r':
        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;

  case 'v':
    return checkKeyword(1, 2, "ar", TOKEN_VAR);
  case 'w':
    return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

static Token identifier() {
  while (isAlpha(peek()) || isdigit(peek()))
    advance();
  return makeToken(identifierType());
}

static Token number() {
  while (isdigit(peek()))
    advance();

  // Look for a fractional part.
  if (peek() == '.' && isdigit(peekNext())) {
    // Consume the ".".
    advance();

    while (isdigit(peek()))
      advance();
  }

  return makeToken(TOKEN_NUMBER);
}

static Token string() {
  while (peek() != '"') {
    if (isAtEnd())
      return errorToken("Unterminated string.");

    if (peek() == '\n')
      ++scanner.line;
    advance();
  }

  // The closing quote.
  advance();
  return makeToken(TOKEN_STRING);
}

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = advance();
  if (isAlpha(c))
    return identifier();
  if (isdigit(c))
    return number();

  switch (c) {
  case '(':
    return makeToken(TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(TOKEN_RIGHT_PAREN);
  case '{':
    return makeToken(TOKEN_LEFT_BRACE);
  case '}':
    return makeToken(TOKEN_RIGHT_BRACE);
  case ';':
    return makeToken(TOKEN_SEMICOLON);
  case ',':
    return makeToken(TOKEN_COMMA);
  case '.':
    return makeToken(TOKEN_DOT);
  case '-':
    return makeToken(TOKEN_MINUS);
  case '+':
    return makeToken(TOKEN_PLUS);
  case '/':
    return makeToken(TOKEN_SLASH);
  case '*':
    return makeToken(TOKEN_STAR);
  case '!':
    return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"':
    return string();
  }

  return errorToken("Unexpected character.");
}

#pragma push_macro("EOF")
#undef EOF

#define STR(s) #s
#define X(token) STR(token),
static const char *tokenTypeNames[] = {TOKEN_TYPES};
#undef X

#pragma pop_macro("EOF")

const char *getTokenTypeName(TokenType type) { return tokenTypeNames[type]; }
