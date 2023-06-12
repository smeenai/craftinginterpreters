#include "LoxClass.h"

#include "LoxInstance.h"

Value LoxClass::call(Interpreter &, const std::vector<Value> &) const {
  return LoxInstance::create(*this);
}
