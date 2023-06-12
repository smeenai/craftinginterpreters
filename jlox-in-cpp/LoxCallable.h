#pragma once

#include <string>
#include <vector>

#include "Interpreter.h"
#include "Value.h"

class LoxCallable {
public:
  virtual ~LoxCallable() = default;
  virtual size_t arity() const = 0;
  virtual Value call(Interpreter &interpreter,
                     const std::vector<Value> &arguments) const = 0;
  virtual std::string str() const = 0;
};
