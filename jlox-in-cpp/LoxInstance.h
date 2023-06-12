#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "LoxClass.h"
#include "RuntimeError.h"
#include "Value.h"

class LoxInstance {
public:
  LoxInstance(const LoxClass &klass) : klass(klass) {}

  std::string str() const { return klass.str() + " instance"; }

  Value get(const Token &name) const {
    auto it = fields.find(name.lexeme);
    if (it != fields.end())
      return it->second;

    throw RuntimeError(name, "Undefined property '" + std::string(name.lexeme) +
                                 "'.");
  }

  void set(const Token &name, Value value) { fields[name.lexeme] = value; }

private:
  const LoxClass &klass;
  std::unordered_map<std::string_view, Value> fields;
};
