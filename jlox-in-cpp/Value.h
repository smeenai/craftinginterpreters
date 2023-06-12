#pragma once

#include <memory>
#include <ostream>
#include <variant>

#include "StringValue.h"

// We need to forward-declare here to avoid circular dependencies.
using Value = std::variant<std::nullptr_t, bool, double, StringValue,
                           std::shared_ptr<const class LoxCallable>,
                           std::shared_ptr<const class LoxInstance>>;

std::ostream &operator<<(std::ostream &o, Value value);
