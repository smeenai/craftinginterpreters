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
  void define(std::string_view name, Value value) { values[name] = value; }

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
    ancestor(distance).values[name.lexeme] = value;
  }

private:
  std::unordered_map<std::string_view, Value> values;

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
