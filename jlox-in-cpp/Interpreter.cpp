#include "Interpreter.h"

#include <iostream>
#include <sstream>

#include "ClockFunction.h"
#include "Error.h"
#include "LoxCallable.h"
#include "LoxFunction.h"
#include "RuntimeError.h"
#include "Token.h"

struct Return {
  Value value;
};

Interpreter::Interpreter() {
  globals.define("clock", std::make_shared<ClockFunction>());
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

void Interpreter::executeBlock(const std::vector<Stmt> &statements,
                               std::shared_ptr<Environment> &&env) {
  EnvironmentGuard envGuard(*this, std::move(env));
  for (Stmt stmt : statements)
    std::visit(*this, stmt);
}

void Interpreter::operator()(const ExpressionStmt *stmt) {
  std::visit(*this, stmt->expr);
}

void Interpreter::operator()(const FunctionStmt *stmt) {
  environment->define(stmt->name.lexeme, std::make_shared<LoxFunction>(*stmt));
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

  throw Return{value};
}

void Interpreter::operator()(const VarStmt *stmt) {
  Value value = nullptr;
  if (stmt->initializer)
    value = std::visit(*this, *stmt->initializer);
  environment->define(stmt->name.lexeme, value);
}

void Interpreter::operator()(const WhileStmt *stmt) {
  while (isTruthy(std::visit(*this, stmt->condition)))
    std::visit(*this, stmt->body);
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
  return environment->get(expr->name);
}

Value Interpreter::operator()(const AssignExpr *expr) {
  Value value = std::visit(*this, expr->value);
  environment->assign(expr->name, value);
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
