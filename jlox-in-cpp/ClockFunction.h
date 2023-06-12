#pragma once

#include <ctime>

#include "LoxCallable.h"

class ClockFunction final : public LoxCallable {
  size_t arity() const override { return 0; }

  Value call(Interpreter &, const std::vector<Value> &) const override {
    return static_cast<double>(std::time(nullptr));
  }

  std::string str() const override { return "<native fn>"; }
};
