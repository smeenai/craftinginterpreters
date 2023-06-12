#include "Error.h"

#include <iostream>
#include <string>

static bool sawError;
static bool sawRuntimeError;

static void report(int line, std::string_view where, std::string_view message) {
  std::cerr << "[line " << line << "] Error" << where << ": " << message
            << "\n";
  sawError = true;
}

void error(int line, std::string_view message) { report(line, "", message); }

void error(const Token &token, std::string_view message) {
  if (token.type == TokenType::TOKEN_EOF)
    report(token.line, " at end", message);
  else
    report(token.line, " at '" + std::string(token.lexeme) + "'", message);
}

void runtimeError(const RuntimeError &error) {
  std::cerr << error.what() << "\n[line " << error.token.line << "]\n";
  sawRuntimeError = true;
}

bool hadError() { return sawError; }

bool hadRuntimeError() { return sawRuntimeError; }

void clearError() { sawError = false; }
