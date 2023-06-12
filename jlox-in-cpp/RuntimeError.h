#pragma once

#include <stdexcept>

#include "Token.h"

class RuntimeError : public std::runtime_error {
public:
  RuntimeError(const Token &token, const char *message)
      : std::runtime_error(message), token(token) {}
  RuntimeError(const Token &token, const std::string &message)
      : std::runtime_error(message), token(token) {}

  const Token &token;
};
