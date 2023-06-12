#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "LoxCallable.h"
#include "LoxFunction.h"
#include "Value.h"

class LoxClass : public LoxCallable {
public:
  LoxClass(std::string_view name, std::shared_ptr<const LoxClass> &&superclass,
           std::unordered_map<std::string_view, LoxFunction> &&methods)
      : name(name), superclass(std::move(superclass)),
        methods(std::move(methods)), initializer(findMethod("init")) {}

  std::string str() const override { return std::string(name); }

  size_t arity() const override {
    return initializer ? initializer->arity() : 0;
  }

  Value call(Interpreter &, const std::vector<Value> &) const override;

  const LoxFunction *findMethod(std::string_view name) const {
    auto it = methods.find(name);
    if (it != methods.end())
      return &it->second;

    return superclass ? superclass->findMethod(name) : nullptr;
  }

private:
  std::string_view name;
  std::shared_ptr<const LoxClass> superclass;
  const std::unordered_map<std::string_view, LoxFunction> methods;
  const LoxFunction *initializer;
};
