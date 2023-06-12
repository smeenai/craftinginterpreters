#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "RuntimeError.h"
#include "Token.h"
#include "Value.h"

class Environment {
public:
  // I'm following the book and taking String instead of Token like get below
  // (which needs Token so it can construct a RuntimeError). I imagine there's a
  // reason for this which will be revealed later.
  void define(std::string_view name, Value value) {
    values[std::string(name)] = value;
  }

  Value get(const Token &name) const {
    auto it = values.find(name.lexeme);
    if (it != values.end())
      return it->second;

    throw RuntimeError(name, "Undefined variable '" + std::string(name.lexeme) +
                                 "'.");
  }

  void assign(const Token &name, Value value) {
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
      it->second = value;
      return;
    }

    throw RuntimeError(name, "Undefined variable '" + std::string(name.lexeme) +
                                 "'.");
  }

private:
  struct string_view_hash {
    using is_transparent = void;
    std::size_t operator()(std::string_view str) const {
      return std::hash<std::string_view>()(str);
    }
  };

  // Right now in Lox.cpp, the interpreter is file-static but REPL input lines
  // (which contain the variable names) are scoped to the loop in runPrompt, so
  // holding a string_view here would lead to dangling references. We could keep
  // the entire input line alive, but that would use more memory than just
  // copying the variable names over. We'll do all our lookups using string_view
  // to avoid any unnecessary copying there.
  // TODO: Reconsider this if it turns out we need to keep the entire REPL input
  // lines alive for some other reason anyway.
  std::unordered_map<std::string, Value, string_view_hash, std::equal_to<>>
      values;
};
