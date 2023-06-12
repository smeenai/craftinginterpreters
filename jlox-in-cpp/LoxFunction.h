#pragma once

#include "Environment.h"
#include "LoxCallable.h"

class LoxFunction : public LoxCallable {
public:
  LoxFunction(const FunctionStmt &declaration,
              const std::shared_ptr<Environment> &closure)
      : declaration(declaration), closure(closure) {}

  size_t arity() const override { return declaration.params.size(); }

  Value call(Interpreter &interpreter,
             const std::vector<Value> &arguments) const override {
    auto env = std::make_shared<Environment>(closure);
    for (size_t i = 0; i < declaration.params.size(); ++i)
      env->define(declaration.params[i].get().lexeme, arguments[i]);

    try {
      interpreter.executeBlock(declaration.body, std::move(env));
    } catch (const Interpreter::Return &returnValue) {
      return returnValue.value;
    }

    return nullptr;
  }

  std::string str() const override {
    return "<fn " + std::string(declaration.name.lexeme) + ">";
  }

private:
  const FunctionStmt &declaration;
  const std::shared_ptr<Environment> closure;
};
