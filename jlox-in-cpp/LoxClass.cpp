#include "LoxClass.h"

#include "LoxInstance.h"

Value LoxClass::call(Interpreter &interpreter,
                     const std::vector<Value> &arguments) const {
  std::shared_ptr<LoxInstance> instance = LoxInstance::create(*this);
  if (initializer)
    initializer->bind(instance)->call(interpreter, arguments);

  return instance;
}
