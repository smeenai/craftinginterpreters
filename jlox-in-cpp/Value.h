#pragma once

#include <ostream>
#include <variant>

#include "StringValue.h"

using Value = std::variant<std::nullptr_t, bool, double, StringValue>;

std::ostream &operator<<(std::ostream &o, Value value);
