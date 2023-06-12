#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RuntimeError.h"
#include "Token.h"
#include "Value.h"

class Environment {
public:
  Environment(const std::shared_ptr<Environment> &enclosing = nullptr)
      : enclosing(enclosing) {}

  // I'm following the book and taking String instead of Token like get below
  // (which needs Token so it can construct a RuntimeError). I imagine there's a
  // reason for this which will be revealed later.
  void define(std::string_view name, Value value) {
    values[std::string(name)] = value;
  }

  Value get(const Token &name) const {
    for (const Environment *env = this; env != nullptr;
         env = env->enclosing.get()) {
      auto it = env->values.find(name.lexeme);
      if (it != env->values.end())
        return it->second;
    }

    throw RuntimeError(name, "Undefined variable '" + std::string(name.lexeme) +
                                 "'.");
  }

  Value getAt(unsigned distance, std::string_view name) const {
    const Environment &env = ancestor(distance);
    // We can't use at because it doesn't allow heterogeneous lookup :(
    auto it = env.values.find(name);
    assert(it != env.values.end());
    return it->second;
  }

  void assign(const Token &name, Value value) {
    for (Environment *env = this; env != nullptr; env = env->enclosing.get()) {
      auto it = env->values.find(name.lexeme);
      if (it != env->values.end()) {
        it->second = value;
        return;
      }
    }

    throw RuntimeError(name, "Undefined variable '" + std::string(name.lexeme) +
                                 "'.");
  }

  void assignAt(unsigned distance, const Token &name, Value value) {
    ancestor(distance).values[std::string(name.lexeme)] = value;
  }

private:
  // This allows heterogenous lookup so we don't have to construct a std::string
  // to look up names: https://stackoverflow.com/a/53530846
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

  std::shared_ptr<Environment> enclosing;

  const Environment &ancestor(unsigned distance) const {
    const Environment *env = this;
    for (unsigned i = 0; i < distance; ++i)
      env = env->enclosing.get();

    return *env;
  }

  // I hope C++23 deducing this will obviate the need for this
  Environment &ancestor(unsigned distance) {
    return const_cast<Environment &>(
        const_cast<const Environment *>(this)->ancestor(distance));
  }
};
