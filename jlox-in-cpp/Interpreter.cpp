#include "Interpreter.h"

#include <iostream>
#include <sstream>
#include <string_view>

#include "ClockFunction.h"
#include "Error.h"
#include "LoxCallable.h"
#include "LoxClass.h"
#include "LoxFunction.h"
#include "LoxInstance.h"
#include "RuntimeError.h"
#include "Token.h"

Interpreter::Interpreter() {
  globals.define("clock", std::make_shared<ClockFunction>());
}

Interpreter::~Interpreter() {
  for (Expr expr : exprStorage)
    std::visit([](auto &&ptr) { delete ptr; }, expr);
  for (Stmt stmt : stmtStorage)
    std::visit([](auto &&ptr) { delete ptr; }, stmt);
}

void Interpreter::interpret(const std::vector<Stmt> &statements) {
  try {
    for (Stmt stmt : statements)
      std::visit(*this, stmt);
  } catch (const RuntimeError &error) {
    runtimeError(error);
  }
}

void Interpreter::operator()(const BlockStmt *stmt) {
  executeBlock(stmt->statements, std::make_shared<Environment>(environment));
}

void Interpreter::operator()(const ClassStmt *stmt) {
  std::shared_ptr<const LoxClass> superclass;
  Value superclassValue = nullptr;
  if (stmt->superclass) {
    superclassValue = (*this)(stmt->superclass);
    if (auto *superclassCallable =
            std::get_if<std::shared_ptr<const LoxCallable>>(&superclassValue))
      superclass =
          std::dynamic_pointer_cast<const LoxClass>(*superclassCallable);
    if (!superclass)
      throw RuntimeError(stmt->superclass->name, "Superclass must be a class.");
  }

  environment->define(stmt->name.lexeme, nullptr);

  std::unordered_map<std::string_view, LoxFunction> methods;
  {
    EnvironmentGuard superGuard(*this, nullptr);
    if (stmt->superclass) {
      environment = std::make_shared<Environment>(environment);
      environment->define("super", superclassValue);
    }

    for (const FunctionStmt *method : stmt->methods)
      methods.emplace(method->name.lexeme,
                      LoxFunction(*method, environment,
                                  method->name.lexeme == "init"
                                      ? FunctionType::INITIALIZER
                                      : FunctionType::NOT_INITIALIZER));
  }

  environment->assign(stmt->name, std::make_shared<const LoxClass>(
                                      stmt->name.lexeme, std::move(superclass),
                                      std::move(methods)));
}

void Interpreter::executeBlock(const std::vector<Stmt> &statements,
                               std::shared_ptr<Environment> &&env) {
  EnvironmentGuard envGuard(*this, std::move(env));
  for (Stmt stmt : statements) {
    std::visit(*this, stmt);
    if (returnStack.back())
      return;
  }
}

void Interpreter::operator()(const ExpressionStmt *stmt) {
  std::visit(*this, stmt->expr);
}

void Interpreter::operator()(const FunctionStmt *stmt) {
  environment->define(stmt->name.lexeme,
                      std::make_shared<LoxFunction>(
                          *stmt, environment, FunctionType::NOT_INITIALIZER));
}

void Interpreter::operator()(const IfStmt *stmt) {
  if (isTruthy(std::visit(*this, stmt->condition)))
    std::visit(*this, stmt->thenBranch);
  else if (stmt->elseBranch)
    std::visit(*this, *stmt->elseBranch);
}

void Interpreter::operator()(const PrintStmt *stmt) {
  Value value = std::visit(*this, stmt->expr);
  std::cout << value << "\n";
}

void Interpreter::operator()(const ReturnStmt *stmt) {
  Value value = nullptr;
  if (stmt->value)
    value = std::visit(*this, *stmt->value);

  returnStack.back() = value;
}

void Interpreter::operator()(const VarStmt *stmt) {
  Value value = nullptr;
  if (stmt->initializer)
    value = std::visit(*this, *stmt->initializer);
  environment->define(stmt->name.lexeme, value);
}

void Interpreter::operator()(const WhileStmt *stmt) {
  while (isTruthy(std::visit(*this, stmt->condition))) {
    std::visit(*this, stmt->body);
    if (returnStack.back())
      return;
  }
}

Value Interpreter::operator()(const LiteralExpr *expr) {
  return std::visit([](auto &&v) -> Value { return v; }, expr->value);
}

Value Interpreter::operator()(const LogicalExpr *expr) {
  Value left = std::visit(*this, expr->left);
  if (isTruthy(left) == (expr->op.type == TokenType::OR))
    return left;
  return std::visit(*this, expr->right);
}

Value Interpreter::operator()(const GetExpr *expr) {
  Value object = std::visit(*this, expr->object);
  if (auto *instance = std::get_if<std::shared_ptr<LoxInstance>>(&object))
    return (*instance)->get(expr->name);

  throw RuntimeError(expr->name, "Only instances have properties.");
}

Value Interpreter::operator()(const SetExpr *expr) {
  Value object = std::visit(*this, expr->object);
  auto *instance = std::get_if<std::shared_ptr<LoxInstance>>(&object);
  if (!instance)
    throw RuntimeError(expr->name, "Only instances have fields.");

  Value value = std::visit(*this, expr->value);
  (*instance)->set(expr->name, value);
  return value;
}

Value Interpreter::operator()(const SuperExpr *expr) {
  unsigned distance = locals.at(expr);
  const LoxClass &superclass = dynamic_cast<const LoxClass &>(
      *std::get<std::shared_ptr<const LoxCallable>>(
          environment->getAt(distance, "super")));
  Value thisValue = environment->getAt(distance - 1, "this");
  const auto &object = std::get<std::shared_ptr<LoxInstance>>(thisValue);

  const LoxFunction *method = superclass.findMethod(expr->method.lexeme);
  if (!method)
    throw RuntimeError(expr->method, "Undefined property '" +
                                         std::string(expr->method.lexeme) +
                                         "'.");

  return method->bind(object);
}

Value Interpreter::operator()(const ThisExpr *expr) {
  return lookUpVariable(expr->keyword, expr);
}

Value Interpreter::operator()(const GroupingExpr *expr) {
  return std::visit(*this, expr->expr);
}

Value Interpreter::operator()(const UnaryExpr *expr) {
  Value right = std::visit(*this, expr->right);

  switch (expr->op.type) {
  case TokenType::BANG:
    return !isTruthy(right);

  case TokenType::MINUS:
    checkNumberOperand(expr->op, right);
    return -std::get<double>(right);

  default:
    __builtin_unreachable();
  }
}

Value Interpreter::operator()(const VariableExpr *expr) {
  return lookUpVariable(expr->name, expr);
}

Value Interpreter::lookUpVariable(const Token &name, Expr expr) const {
  auto it = locals.find(expr);
  if (it != locals.end())
    return environment->getAt(it->second, name.lexeme);

  return globals.get(name);
}

Value Interpreter::operator()(const AssignExpr *expr) {
  Value value = std::visit(*this, expr->value);

  auto it = locals.find(expr);
  if (it != locals.end())
    environment->assignAt(it->second, expr->name, value);
  else
    globals.assign(expr->name, value);

  return value;
}

Value Interpreter::operator()(const BinaryExpr *expr) {
  Value left = std::visit(*this, expr->left);
  Value right = std::visit(*this, expr->right);

  switch (expr->op.type) {
  case TokenType::BANG_EQUAL:
    return left != right;

  case TokenType::EQUAL_EQUAL:
    return left == right;

  case TokenType::GREATER:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) > std::get<double>(right);

  case TokenType::GREATER_EQUAL:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) >= std::get<double>(right);

  case TokenType::LESS:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) < std::get<double>(right);

  case TokenType::LESS_EQUAL:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) <= std::get<double>(right);

  case TokenType::MINUS:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) - std::get<double>(right);

  case TokenType::PLUS:
    if (std::holds_alternative<double>(left) &&
        std::holds_alternative<double>(right))
      return std::get<double>(left) + std::get<double>(right);

    if (std::holds_alternative<StringValue>(left) &&
        std::holds_alternative<StringValue>(right))
      return StringValue(std::get<StringValue>(left),
                         std::get<StringValue>(right));

    throw RuntimeError(expr->op,
                       "Operands must be two numbers or two strings.");

  case TokenType::SLASH:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) / std::get<double>(right);

  case TokenType::STAR:
    checkNumberOperands(expr->op, left, right);
    return std::get<double>(left) * std::get<double>(right);

  default:
    __builtin_unreachable();
  }
}

Value Interpreter::operator()(const CallExpr *expr) {
  Value callee = std::visit(*this, expr->callee);

  std::vector<Value> arguments;
  for (const Expr argument : expr->arguments)
    arguments.push_back(std::visit(*this, argument));

  const auto *function =
      std::get_if<std::shared_ptr<const LoxCallable>>(&callee);
  if (!function)
    throw RuntimeError(expr->paren, "Can only call functions and classes.");

  if (arguments.size() != (*function)->arity())
    throw RuntimeError(expr->paren, std::string("Expected ") +
                                        std::to_string((*function)->arity()) +
                                        " arguments but got " +
                                        std::to_string(arguments.size()) + ".");

  return (*function)->call(*this, arguments);
}

bool Interpreter::isTruthy(Value value) {
  if (std::holds_alternative<std::nullptr_t>(value))
    return false;
  if (bool *b = std::get_if<bool>(&value))
    return *b;
  return true;
}

void Interpreter::checkNumberOperand(const Token &token, Value value) {
  if (!std::holds_alternative<double>(value))
    throw new RuntimeError(token, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token &token, Value left,
                                      Value right) {
  if (!std::holds_alternative<double>(left) ||
      !std::holds_alternative<double>(right))
    throw new RuntimeError(token, "Operands must be numbers.");
}
