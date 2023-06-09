#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Environment.h"
#include "Expr.h"
#include "Stmt.h"
#include "Value.h"

class Interpreter {
public:
  Interpreter();
  ~Interpreter();
  void resolve(Expr expr, unsigned distance) { locals[expr] = distance; }
  void interpret(const std::vector<Stmt> &statements);

  void operator()(const BlockStmt *stmt);
  void operator()(const ClassStmt *stmt);
  void operator()(const ExpressionStmt *stmt);
  void operator()(const FunctionStmt *stmt);
  void operator()(const IfStmt *stmt);
  void operator()(const PrintStmt *stmt);
  void operator()(const ReturnStmt *stmt);
  void operator()(const VarStmt *stmt);
  void operator()(const WhileStmt *stmt);

  Value operator()(const LiteralExpr *expr);
  Value operator()(const LogicalExpr *expr);
  Value operator()(const GetExpr *expr);
  Value operator()(const SetExpr *expr);
  Value operator()(const SuperExpr *expr);
  Value operator()(const ThisExpr *expr);
  Value operator()(const GroupingExpr *expr);
  Value operator()(const UnaryExpr *expr);
  Value operator()(const VariableExpr *expr);
  Value operator()(const AssignExpr *expr);
  Value operator()(const BinaryExpr *expr);
  Value operator()(const CallExpr *expr);

  void executeBlock(const std::vector<Stmt> &statements,
                    std::shared_ptr<Environment> &&env);

  class ReturnStackGuard {
  public:
    [[nodiscard]] ReturnStackGuard(Interpreter &interpreter)
        : interpreter(interpreter) {
      interpreter.returnStack.emplace_back();
    }

    ~ReturnStackGuard() { interpreter.returnStack.pop_back(); }

    const std::optional<Value> &peek() {
      return interpreter.returnStack.back();
    }

  private:
    Interpreter &interpreter;
  };

  // Own all tokens and AST nodes to avoid any lifetime issues.
  std::vector<Token> &newTokenStorage() { return tokenStorage.emplace_back(); }
  std::vector<Expr> exprStorage;
  std::vector<Stmt> stmtStorage;

private:
  std::shared_ptr<Environment> environment = std::make_shared<Environment>();
  Environment &globals = *environment;
  std::unordered_map<Expr, unsigned> locals;

  std::vector<std::optional<Value>> returnStack = {{}};

  // A vector of vectors would be okay, since the inner vectors should be moved
  // instead of copied when resizing the outer vector (thus preserving
  // references to their elements), but a deque avoids copying on growth.
  std::deque<std::vector<Token>> tokenStorage;

  class EnvironmentGuard {
  public:
    [[nodiscard]] EnvironmentGuard(Interpreter &interpreter,
                                   std::shared_ptr<Environment> &&newEnv)
        : interpreter(interpreter), oldEnv(std::move(interpreter.environment)) {
      if (newEnv)
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

  Value lookUpVariable(const Token &name, Expr expr) const;

  static bool isTruthy(Value value);
  static void checkNumberOperand(const Token &token, Value value);
  static void checkNumberOperands(const Token &token, Value left, Value right);
};
