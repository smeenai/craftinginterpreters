#include "Resolver.h"

#include <unordered_set>
#include <variant>

#include "Error.h"

void Resolver::resolve(const std::vector<Stmt> &statements) {
  for (Stmt stmt : statements)
    std::visit(*this, stmt);
}

void Resolver::operator()(const BlockStmt *stmt) {
  ScopeGuard scopeGuard(*this);
  resolve(stmt->statements);
}

void Resolver::operator()(const ClassStmt *stmt) {
  SaveAndRestore currentClassGuard(currentClass, ClassType::CLASS);

  declare(stmt->name);
  define(stmt->name);

  if (stmt->superclass) {
    if (stmt->name.lexeme == stmt->superclass->name.lexeme)
      error(stmt->superclass->name, "A class can't inherit from itself.");

    currentClass = ClassType::SUBCLASS;
    (*this)(stmt->superclass);
  }

  ScopeGuard superGuard(*this, stmt->superclass);
  if (stmt->superclass)
    scopes.back().emplace("super", true);

  ScopeGuard scopeGuard(*this);
  scopes.back().emplace("this", true);

  std::unordered_set<std::string_view> methodNames;
  for (const FunctionStmt *method : stmt->methods) {
    // This is an intentional divergence from jlox, which permits multiple
    // methods with the same name and just uses the last one.
    auto [_, wasInserted] = methodNames.insert(method->name.lexeme);
    if (!wasInserted)
      error(method->name, "Already a method with this name in this class.");

    resolveFunction(method, method->name.lexeme == "init"
                                ? FunctionType::INITIALIZER
                                : FunctionType::METHOD);
  }
}

void Resolver::operator()(const ExpressionStmt *stmt) {
  std::visit(*this, stmt->expr);
}

void Resolver::operator()(const FunctionStmt *stmt) {
  declare(stmt->name);
  define(stmt->name);
  resolveFunction(stmt, FunctionType::FUNCTION);
}

void Resolver::operator()(const IfStmt *stmt) {
  std::visit(*this, stmt->condition);
  std::visit(*this, stmt->thenBranch);
  if (stmt->elseBranch)
    std::visit(*this, *stmt->elseBranch);
}

void Resolver::operator()(const PrintStmt *stmt) {
  std::visit(*this, stmt->expr);
}

void Resolver::operator()(const ReturnStmt *stmt) {
  if (currentFunction == FunctionType::NONE)
    error(stmt->keyword, "Can't return from top-level code");

  if (stmt->value) {
    if (currentFunction == FunctionType::INITIALIZER)
      error(stmt->keyword, "Can't return a value from an initializer.");

    std::visit(*this, *stmt->value);
  }
}

void Resolver::operator()(const VarStmt *stmt) {
  declare(stmt->name);
  if (stmt->initializer)
    std::visit(*this, *stmt->initializer);
  define(stmt->name);
}

void Resolver::operator()(const WhileStmt *stmt) {
  std::visit(*this, stmt->condition);
  std::visit(*this, stmt->body);
}

void Resolver::operator()(const AssignExpr *expr) {
  std::visit(*this, expr->value);
  resolveLocal(expr, expr->name);
}

void Resolver::operator()(const BinaryExpr *expr) {
  std::visit(*this, expr->left);
  std::visit(*this, expr->right);
}

void Resolver::operator()(const CallExpr *expr) {
  std::visit(*this, expr->callee);
  for (Expr argument : expr->arguments)
    std::visit(*this, argument);
}

void Resolver::operator()(const GetExpr *expr) {
  std::visit(*this, expr->object);
}

void Resolver::operator()(const GroupingExpr *expr) {
  std::visit(*this, expr->expr);
}

void Resolver::operator()(const LiteralExpr *) {}

void Resolver::operator()(const LogicalExpr *expr) {
  std::visit(*this, expr->left);
  std::visit(*this, expr->right);
}

void Resolver::operator()(const SetExpr *expr) {
  std::visit(*this, expr->value);
  std::visit(*this, expr->object);
}

void Resolver::operator()(const SuperExpr *expr) {
  if (currentClass == ClassType::NONE)
    error(expr->keyword, "Can't use 'super' outside of a class.");
  else if (currentClass != ClassType::SUBCLASS)
    error(expr->keyword, "Can't use 'super' in a class with no superclass.");

  resolveLocal(expr, expr->keyword);
}

void Resolver::operator()(const ThisExpr *expr) {
  if (currentClass == ClassType::NONE)
    error(expr->keyword, "Can't use 'this' outside of a class.");

  resolveLocal(expr, expr->keyword);
}

void Resolver::operator()(const UnaryExpr *expr) {
  std::visit(*this, expr->right);
}

void Resolver::operator()(const VariableExpr *expr) {
  if (!scopes.empty())
    if (auto it = scopes.back().find(expr->name.lexeme);
        it != scopes.back().end() && !it->second)
      error(expr->name, "Can't read local variable in its own initializer.");

  resolveLocal(expr, expr->name);
}

void Resolver::declare(const Token &name) {
  if (scopes.empty())
    return;

  auto [_, wasInserted] = scopes.back().emplace(name.lexeme, false);
  if (!wasInserted)
    error(name, "Already a variable with this name in this scope.");
}

void Resolver::define(const Token &name) {
  if (!scopes.empty())
    scopes.back()[name.lexeme] = true;
}

void Resolver::resolveLocal(Expr expr, const Token &name) {
  unsigned distance = 0;
  for (auto it = scopes.crbegin(); it != scopes.crend(); ++it, ++distance) {
    if (it->contains(name.lexeme)) {
      interpreter.resolve(expr, distance);
      return;
    }
  }
}

void Resolver::resolveFunction(const FunctionStmt *function,
                               FunctionType type) {
  SaveAndRestore currentFunctionGuard(currentFunction, type);
  ScopeGuard scopeGuard(*this);
  for (const Token &param : function->params) {
    declare(param);
    define(param);
  }
  resolve(function->body);
}
