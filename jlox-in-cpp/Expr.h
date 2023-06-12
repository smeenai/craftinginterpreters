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
                 const struct CallExpr *, const struct FunctionExpr *,
                 const struct GroupingExpr *, const struct LiteralExpr *,
                 const struct LogicalExpr *, const struct UnaryExpr *,
                 const struct VariableExpr *>;

// This is a dirty hack. Expr and Stmt types have a circular dependency with the
// introduction of FunctionExpr. Putting them in the same header would be the
// clean way to resolve that but require a lot of churn. Instead, we include
// Stmt.h here only after defining Expr, so that it retains access to it.
#include "Stmt.h"

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

struct FunctionExpr {
  const Token *name; // can be nullptr for anonymous functions
  const std::vector<std::reference_wrapper<const Token>> params;
  const std::vector<Stmt> body;
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

struct UnaryExpr {
  const Token &op;
  const Expr right;
};

struct VariableExpr {
  const Token &name;
};
