#pragma once

#include <ostream>
#include <variant>

// This is equivalent to the LiteralExpr variant right now, but I suspect it
// won't be after subsequent chapters, so I'm keeping it separate.
using Value = std::variant<std::nullptr_t, bool, double, std::string_view>;

std::ostream &operator<<(std::ostream &o, Value value);
