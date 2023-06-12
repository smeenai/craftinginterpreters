#pragma once

#include <optional>
#include <variant>

#include "Expr.h"
#include "Token.h"

// See Expr.h for why we use std::variant instead of a class hierarchy.

using Stmt =
    std::variant<const struct BlockStmt *, const struct ExpressionStmt *,
                 const struct PrintStmt *, const struct VarStmt *>;

struct BlockStmt {
  const std::vector<Stmt> statements;
};

struct ExpressionStmt {
  const Expr expr;
};

struct PrintStmt {
  const Expr expr;
};

struct VarStmt {
  const Token &name;
  const std::optional<Expr> initializer;
};
