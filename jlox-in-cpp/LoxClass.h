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
  LoxClass(std::string_view name,
           std::unordered_map<std::string_view, LoxFunction> &&methods)
      : name(name), methods(std::move(methods)) {}

  std::string str() const override { return std::string(name); }

  size_t arity() const override { return 0; }

  Value call(Interpreter &, const std::vector<Value> &) const override;

  const LoxFunction *findMethod(std::string_view name) const {
    auto it = methods.find(name);
    return it != methods.end() ? &it->second : nullptr;
  }

private:
  std::string_view name;
  const std::unordered_map<std::string_view, LoxFunction> methods;
};
