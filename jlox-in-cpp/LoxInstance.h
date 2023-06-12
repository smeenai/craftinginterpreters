#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "LoxClass.h"
#include "LoxFunction.h"
#include "RuntimeError.h"
#include "Value.h"

class LoxInstance : public std::enable_shared_from_this<LoxInstance> {
public:
  static Value create(const LoxClass &klass) {
    // make_shared isn't happy with private constructors without some help :(
    struct make_shared_enabler : public LoxInstance {
      make_shared_enabler(const LoxClass &klass) : LoxInstance(klass) {}
    };
    return std::make_shared<make_shared_enabler>(klass);
  }

  std::string str() const { return klass.str() + " instance"; }

  Value get(const Token &name) const {
    auto it = fields.find(name.lexeme);
    if (it != fields.end())
      return it->second;

    const LoxFunction *method = klass.findMethod(name.lexeme);
    if (method)
      return method->bind(const_cast<LoxInstance *>(this)->shared_from_this());

    throw RuntimeError(name, "Undefined property '" + std::string(name.lexeme) +
                                 "'.");
  }

  void set(const Token &name, Value value) { fields[name.lexeme] = value; }

private:
  const LoxClass &klass;
  std::unordered_map<std::string_view, Value> fields;

  // Ensure that any instance is managed by a shared_ptr so that we can safely
  // use shared_from_this.
  LoxInstance(const LoxClass &klass) : klass(klass) {}
};
