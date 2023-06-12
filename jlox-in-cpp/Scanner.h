#pragma once

#include <string_view>
#include <vector>

#include "Token.h"

class Scanner {
  const std::string_view source;
  std::vector<Token> tokens;
  size_t start = 0;
  size_t current = 0;
  unsigned line = 1;

  void scanToken();

  void identifier();

  void number(); 

  void string();

  bool match(char expected);

  char peek();

  char peekNext();

  bool isAtEnd();

  char advance();

  void addToken(TokenType type);

  void addToken(std::string_view literal);

  void addToken(double literal);

	std::string_view currentLexeme();

public:
  Scanner(std::string_view source) : source(source) {}

  const std::vector<Token> &scanTokens();
};
