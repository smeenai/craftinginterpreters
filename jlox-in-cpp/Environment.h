#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RuntimeError.h"
#include "Token.h"
#include "Value.h"

class Environment {
public:
  // I'm following the book and taking String instead of Token like get below
  // (which needs Token so it can construct a RuntimeError). I imagine there's a
  // reason for this which will be revealed later.
  void define(std::string_view name, Value value) {
    scopes.back()[std::string(name)] = value;
  }

  Value get(const Token &name) const {
    for (auto scopeIt = scopes.rbegin(); scopeIt != scopes.rend(); ++scopeIt) {
      auto it = scopeIt->find(name.lexeme);
      if (it != scopeIt->end())
        return it->second;
    }

    throw RuntimeError(name, "Undefined variable '" + std::string(name.lexeme) +
                                 "'.");
  }

  void assign(const Token &name, Value value) {
    for (auto scopeIt = scopes.rbegin(); scopeIt != scopes.rend(); ++scopeIt) {
      auto it = scopeIt->find(name.lexeme);
      if (it != scopeIt->end()) {
        it->second = value;
        return;
      }
    }

    throw RuntimeError(name, "Undefined variable '" + std::string(name.lexeme) +
                                 "'.");
  }

  // A simple RAII wrapper to manage environment scopes.
  class ScopeGuard {
  public:
    ~ScopeGuard() { environment.scopes.pop_back(); }
    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard(ScopeGuard &&) = delete;
    ScopeGuard &operator=(const ScopeGuard &) = delete;
    ScopeGuard &operator=(ScopeGuard &&) = delete;

  private:
    friend class Environment;
    ScopeGuard(Environment &environment) : environment(environment) {
      environment.scopes.emplace_back();
    }
    Environment &environment;
  };

  [[nodiscard]] ScopeGuard addScope() { return ScopeGuard(*this); }

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
  std::vector<
      std::unordered_map<std::string, Value, string_view_hash, std::equal_to<>>>
      scopes = {{}};
};
