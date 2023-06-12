#pragma once

#include "Environment.h"
#include "LoxCallable.h"

class LoxFunction : public LoxCallable {
public:
  LoxFunction(const FunctionStmt &declaration) : declaration(declaration) {}

  size_t arity() const override { return declaration.params.size(); }

  Value call(Interpreter &interpreter,
             const std::vector<Value> &arguments) const override {
    auto env = std::make_shared<Environment>(interpreter.globals);
    for (size_t i = 0; i < declaration.params.size(); ++i)
      env->define(declaration.params[i].get().lexeme, arguments[i]);

    interpreter.executeBlock(declaration.body, std::move(env));
    return nullptr;
  }

  std::string str() const override {
    return "<fn " + std::string(declaration.name.lexeme) + ">";
  }

private:
  const FunctionStmt &declaration;
};
