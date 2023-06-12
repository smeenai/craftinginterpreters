#pragma once

#include <cstddef>
#include <string_view>
#include <variant>
#include <vector>

#include "Token.h"

// The book uses a class hierarchy with a generic virtual method, whereas C++
// virtual methods can't be templated. std::variant is a more natural fit for
// this anyway, so that works out.

using Expr =
    std::variant<const struct AssignExpr *, const struct BinaryExpr *,
                 const struct CallExpr *, const struct GetExpr *,
                 const struct GroupingExpr *, const struct LiteralExpr *,
                 const struct LogicalExpr *, const struct SetExpr *,
                 const struct ThisExpr *, const struct UnaryExpr *,
                 const struct VariableExpr *>;

struct AssignExpr {
  const Token &name;
  Expr value;
};

struct BinaryExpr {
  const Expr left;
  const Token &op;
  const Expr right;
};

struct CallExpr {
  const Expr callee;
  const Token &paren;
  const std::vector<Expr> arguments;
};

struct GetExpr {
  const Expr object;
  const Token &name;
};

struct GroupingExpr {
  const Expr expr;
};

struct LiteralExpr {
  const std::variant<double, std::string_view, bool, std::nullptr_t> value;
};

struct LogicalExpr {
  const Expr left;
  const Token &op;
  const Expr right;
};

struct SetExpr {
  const Expr object;
  const Token &name;
  const Expr value;
};

struct ThisExpr {
  const Token &keyword;
};

struct UnaryExpr {
  const Token &op;
  const Expr right;
};

struct VariableExpr {
  const Token &name;
};
