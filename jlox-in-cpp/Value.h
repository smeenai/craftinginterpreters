#pragma once

#include <memory>
#include <ostream>
#include <variant>

#include "StringValue.h"

// We need to forward-declare LoxCallable here to avoid a circular header
// dependency, since LoxCallable needs Value.
using Value = std::variant<std::nullptr_t, bool, double, StringValue,
                           std::shared_ptr<const class LoxCallable>>;

std::ostream &operator<<(std::ostream &o, Value value);
