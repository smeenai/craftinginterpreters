#pragma once

#include <variant>

#include "Expr.h"

// See Expr.h for why we use std::variant instead of a class hierarchy.

using Stmt =
    std::variant<const struct ExpressionStmt *, const struct PrintStmt *>;

struct ExpressionStmt {
  const Expr expr;
};

struct PrintStmt {
  const Expr expr;
};
