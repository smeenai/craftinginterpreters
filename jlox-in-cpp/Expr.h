#pragma once

#include <cstddef>
#include <string_view>
#include <variant>

#include "Token.h"

// The book uses a class hierarchy with a generic virtual method, whereas C++
// virtual methods can't be templated. std::variant is a more natural fit for
// this anyway, so that works out.

using Expr = std::variant<struct BinaryExpr *, struct GroupingExpr *,
                          struct LiteralExpr *, struct UnaryExpr *>;

struct BinaryExpr {
  const Expr left;
  const Token &op;
  const Expr right;
};

struct GroupingExpr {
  const Expr expr;
};

struct LiteralExpr {
  const std::variant<double, std::string_view, bool, std::nullptr_t> value;
};

struct UnaryExpr {
  const Token &op;
  const Expr right;
};
