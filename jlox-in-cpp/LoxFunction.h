#pragma once

#include <memory>

#include "Environment.h"
#include "LoxCallable.h"

enum class FunctionType : bool {
  NOT_INITIALIZER,
  INITIALIZER,
};

class LoxFunction : public LoxCallable {
public:
  LoxFunction(const FunctionStmt &declaration,
              const std::shared_ptr<Environment> &closure, FunctionType type)
      : declaration(declaration), closure(closure),
        isInitializer(type == FunctionType::INITIALIZER) {}

  std::shared_ptr<const LoxFunction>
  bind(std::shared_ptr<class LoxInstance> instance) const {
    auto env = std::make_shared<Environment>(closure);
    env->define("this", std::move(instance));
    return std::make_shared<const LoxFunction>(
        declaration, env,
        isInitializer ? FunctionType::INITIALIZER
                      : FunctionType::NOT_INITIALIZER);
  }

  size_t arity() const override { return declaration.params.size(); }

  Value call(Interpreter &interpreter,
             const std::vector<Value> &arguments) const override {
    auto env = std::make_shared<Environment>(closure);
    for (size_t i = 0; i < declaration.params.size(); ++i)
      env->define(declaration.params[i].get().lexeme, arguments[i]);

    Interpreter::ReturnStackGuard returnStackGuard(interpreter);
    interpreter.executeBlock(declaration.body, std::move(env));
    if (isInitializer)
      return closure->getAt(0, "this");
    return returnStackGuard.peek() ? *returnStackGuard.peek() : nullptr;
  }

  std::string str() const override {
    return "<fn " + std::string(declaration.name.lexeme) + ">";
  }

private:
  const FunctionStmt &declaration;
  const std::shared_ptr<Environment> closure;
  bool isInitializer;
};
