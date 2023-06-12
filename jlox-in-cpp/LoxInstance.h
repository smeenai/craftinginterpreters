#pragma once

#include <string>

#include "LoxClass.h"

class LoxInstance {
public:
  LoxInstance(const LoxClass &klass) : klass(klass) {}

  std::string str() const { return klass.str() + " instance"; }

private:
  const LoxClass &klass;
};
