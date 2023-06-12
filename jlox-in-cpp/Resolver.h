#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>

#include "Expr.h"
#include "Interpreter.h"
#include "Stmt.h"

class Resolver {
public:
  Resolver(Interpreter &interpreter) : interpreter(interpreter) {}
  void resolve(const std::vector<Stmt> &statements);

  void operator()(const BlockStmt *stmt);
  void operator()(const ClassStmt *stmt);
  void operator()(const ExpressionStmt *stmt);
  void operator()(const FunctionStmt *stmt);
  void operator()(const IfStmt *stmt);
  void operator()(const PrintStmt *stmt);
  void operator()(const ReturnStmt *stmt);
  void operator()(const VarStmt *stmt);
  void operator()(const WhileStmt *stmt);

  void operator()(const AssignExpr *expr);
  void operator()(const BinaryExpr *expr);
  void operator()(const CallExpr *expr);
  void operator()(const GetExpr *expr);
  void operator()(const GroupingExpr *expr);
  void operator()(const LiteralExpr *expr);
  void operator()(const LogicalExpr *expr);
  void operator()(const SetExpr *expr);
  void operator()(const SuperExpr *expr);
  void operator()(const ThisExpr *expr);
  void operator()(const UnaryExpr *expr);
  void operator()(const VariableExpr *expr);

private:
  Interpreter &interpreter;
  std::vector<std::unordered_map<std::string_view, bool>> scopes;

  enum class ClassType {
    NONE,
    CLASS,
    SUBCLASS,
  };

  ClassType currentClass = ClassType::NONE;

  enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD,
  };

  FunctionType currentFunction = FunctionType::NONE;

  class ScopeGuard {
  public:
    [[nodiscard]] ScopeGuard(Resolver &resolver, bool condition = true)
        : resolver(resolver), condition(condition) {
      if (condition)
        resolver.scopes.emplace_back();
    }

    ~ScopeGuard() {
      if (condition)
        resolver.scopes.pop_back();
    }

  private:
    Resolver &resolver;
    bool condition;
  };

  template <class T> class SaveAndRestore {
  public:
    [[nodiscard]] SaveAndRestore(T &location, T value)
        : location(location), saved(location) {
      location = value;
    }
    ~SaveAndRestore() { location = saved; }

  private:
    T &location;
    T saved;
  };

  void declare(const Token &name);
  void define(const Token &name);

  void resolveLocal(Expr expr, const Token &name);
  void resolveFunction(const FunctionStmt *function, FunctionType type);
};
