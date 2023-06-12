#include "Scanner.h"

#include <charconv>
#include <cstdlib>
#include <unordered_map>

#include "Error.h"

const std::vector<Token> &Scanner::scanTokens() {
  while (!isAtEnd()) {
    // We are at the beginning of the next lexeme.
    start = current;
    scanToken();
  }

  tokens.emplace_back(TokenType::TOKEN_EOF, "", line);
  return tokens;
}

void Scanner::scanToken() {
  char c = advance();
  switch (c) {
  case '(':
    addToken(TokenType::LEFT_PAREN);
    break;
  case ')':
    addToken(TokenType::RIGHT_PAREN);
    break;
  case '{':
    addToken(TokenType::LEFT_BRACE);
    break;
  case '}':
    addToken(TokenType::RIGHT_BRACE);
    break;
  case ',':
    addToken(TokenType::COMMA);
    break;
  case '.':
    addToken(TokenType::DOT);
    break;
  case '-':
    addToken(TokenType::MINUS);
    break;
  case '+':
    addToken(TokenType::PLUS);
    break;
  case ';':
    addToken(TokenType::SEMICOLON);
    break;
  case '*':
    addToken(TokenType::STAR);
    break;
  case '!':
    addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    break;
  case '=':
    addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    break;
  case '<':
    addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    break;
  case '>':
    addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
    break;

  case '/':
    if (match('/')) {
      // A comment goes until the end of the line.
      while (peek() != '\n' && !isAtEnd())
        advance();
    } else {
      addToken(TokenType::SLASH);
    }
    break;

  case ' ':
  case '\r':
  case '\t':
    // Ignore whitespace.
    break;

  case '\n':
    ++line;
    break;

  case '"':
    string();
    break;

  default:
    if (std::isdigit(c))
      number();
    else if (std::isalpha(c) || c == '_')
      identifier();
    else
      error(line, "Unexpected character.");
    break;
  }
}

void Scanner::identifier() {
  static const std::unordered_map<std::string_view, TokenType> keywords = {
      {"and", TokenType::AND},       {"class", TokenType::CLASS},
      {"else", TokenType::ELSE},     {"false", TokenType::FALSE},
      {"for", TokenType::FOR},       {"fun", TokenType::FUN},
      {"if", TokenType::IF},         {"nil", TokenType::NIL},
      {"or", TokenType::OR},         {"print", TokenType::PRINT},
      {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
      {"this", TokenType::THIS},     {"true", TokenType::TRUE},
      {"var", TokenType::VAR},       {"while", TokenType::WHILE},
  };

  auto isAlphaNumeric = [](char c) {
    return std::isalpha(c) || std::isdigit(c) || c == '_';
  };

  while (isAlphaNumeric(peek()))
    advance();

	auto it = keywords.find(currentLexeme());
  if (it == keywords.end())
    addToken(TokenType::IDENTIFIER);
  else
    addToken(it->second);
}

void Scanner::number() {
  while (std::isdigit(peek()))
    advance();

  // Look for a fractional part.
  if (peek() == '.' && std::isdigit(peekNext())) {
    // Consume the "."
    advance();

    while (std::isdigit(peek()))
      advance();
  }

  // libc++ doesn't implement decimal from_chars yet, and strtod doesn't accept
  // an end point, so we have to manually copy it out.
  std::string numberString(&source.at(start), current - start);
  addToken(std::strtod(numberString.data(), nullptr));
}

void Scanner::string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      ++line;
    advance();
  }

  if (isAtEnd()) {
    error(line, "Unterminated string.");
    return;
  }

  // The closing ".
  advance();

  // Trim the surrounding quotes.
  addToken(source.substr(start + 1, current - start - 2));
}

bool Scanner::match(char expected) {
  if (isAtEnd())
    return false;
  if (peek() != expected)
    return false;

  ++current;
  return true;
}

char Scanner::peek() {
  if (isAtEnd())
    return '\0';
  return source.at(current);
}

char Scanner::peekNext() {
  if (current + 1 >= source.length())
    return '\0';
  return source.at(current + 1);
}

bool Scanner::isAtEnd() { return current >= source.length(); }

char Scanner::advance() { return source.at(current++); }

void Scanner::addToken(TokenType type) {
  tokens.emplace_back(type, currentLexeme(), line);
}

void Scanner::addToken(std::string_view literal) {
  tokens.emplace_back(TokenType::STRING, currentLexeme(), literal, line);
}

void Scanner::addToken(double literal) {
  tokens.emplace_back(TokenType::NUMBER, currentLexeme(), literal, line);
}

std::string_view Scanner::currentLexeme() {
  return source.substr(start, current - start);
}
