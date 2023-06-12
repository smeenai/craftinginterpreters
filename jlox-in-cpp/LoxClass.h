#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "LoxCallable.h"
#include "LoxFunction.h"

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

// I'm pulling some shenanigans here. LoxClass and LoxInstance need complete
// definitions of each other, so we have a circular dependency. You'd normally
// break that by putting things in source files instead of headers, but
// including a header later essentially has the same effect, and I'm lazy.
#include "LoxInstance.h"

inline Value LoxClass::call(Interpreter &, const std::vector<Value> &) const {
  // A forward declaration is needed to avoid circular header dependencies.
  return std::make_shared<class LoxInstance>(*this);
}
