#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Environment.h"
#include "Expr.h"
#include "Stmt.h"
#include "Value.h"

class Interpreter {
public:
  void interpret(const std::vector<Stmt> &statements);

  void operator()(const BlockStmt *stmt);
  void operator()(const ExpressionStmt *stmt);
  void operator()(const IfStmt *stmt);
  void operator()(const PrintStmt *stmt);
  void operator()(const VarStmt *stmt);
  void operator()(const WhileStmt *stmt);

  Value operator()(const LiteralExpr *expr);
  Value operator()(const LogicalExpr *expr);
  Value operator()(const GroupingExpr *expr);
  Value operator()(const UnaryExpr *expr);
  Value operator()(const VariableExpr *expr);
  Value operator()(const AssignExpr *expr);
  Value operator()(const BinaryExpr *expr);

private:
  std::shared_ptr<Environment> environment = std::make_shared<Environment>();

  class EnvironmentGuard {
  public:
    [[nodiscard]] EnvironmentGuard(Interpreter &interpreter,
                                   std::shared_ptr<Environment> &&newEnv)
        : interpreter(interpreter), oldEnv(std::move(interpreter.environment)) {
      interpreter.environment = std::move(newEnv);
    }

    ~EnvironmentGuard() { interpreter.environment = std::move(oldEnv); }

    EnvironmentGuard(const EnvironmentGuard &) = delete;
    EnvironmentGuard(EnvironmentGuard &&) = delete;
    EnvironmentGuard &operator=(const EnvironmentGuard &) = delete;
    EnvironmentGuard &operator=(EnvironmentGuard &&) = delete;

  private:
    Interpreter &interpreter;
    std::shared_ptr<Environment> oldEnv;
  };

  void executeBlock(const std::vector<Stmt> &statements,
                    std::shared_ptr<Environment> &&env);

  static bool isTruthy(Value value);
  static void checkNumberOperand(const Token &token, Value value);
  static void checkNumberOperands(const Token &token, Value left, Value right);
};
