#pragma once

#include <functional>
#include <optional>
#include <variant>

#include "Expr.h"
#include "Token.h"

// See Expr.h for why we use std::variant instead of a class hierarchy.

using Stmt = std::variant<const struct BlockStmt *, const struct ClassStmt *,
                          const struct ExpressionStmt *,
                          const struct FunctionStmt *, const struct IfStmt *,
                          const struct PrintStmt *, const struct ReturnStmt *,
                          const struct VarStmt *, const struct WhileStmt *>;

struct BlockStmt {
  const std::vector<Stmt> statements;
};

struct ClassStmt {
  const Token &name;
  const VariableExpr *superclass; // can be nullptr to indicate no parent
  const std::vector<const FunctionStmt *> methods;
};

struct ExpressionStmt {
  const Expr expr;
};

struct FunctionStmt {
  const Token &name;
  const std::vector<std::reference_wrapper<const Token>> params;
  const std::vector<Stmt> body;
};

struct IfStmt {
  const Expr condition;
  const Stmt thenBranch;
  const std::optional<Stmt> elseBranch;
};

struct PrintStmt {
  const Expr expr;
};

struct ReturnStmt {
  const Token &keyword;
  const std::optional<Expr> value;
};

struct VarStmt {
  const Token &name;
  const std::optional<Expr> initializer;
};

struct WhileStmt {
  const Expr condition;
  const Stmt body;
};
