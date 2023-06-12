#include "Interpreter.h"

#include <iostream>
#include <sstream>

#include "Error.h"
#include "RuntimeError.h"
#include "Token.h"

void Interpreter::interpret(Expr expr) {
  try {
    Value value = std::visit(*this, expr);
    std::cout << stringify(value) << "\n";
  } catch (const RuntimeError &error) {
    runtimeError(error);
  }
}

auto Interpreter::operator()(const LiteralExpr *expr) -> Value {
  return std::visit([](auto &&v) -> Value { return v; }, expr->value);
}

auto Interpreter::operator()(const GroupingExpr *expr) -> Value {
  return std::visit(*this, expr->expr);
}

auto Interpreter::operator()(const UnaryExpr *expr) -> Value {
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

auto Interpreter::operator()(const BinaryExpr *expr) -> Value {
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

    if (std::holds_alternative<std::string_view>(left) &&
        std::holds_alternative<std::string_view>(right))
      return saveString(std::string(std::get<std::string_view>(left)) +
                        std::string(std::get<std::string_view>(right)));

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

std::string Interpreter::stringify(Value value) {
  // This is identical to AstPrinter's LiteralExpr logic right now, but it might
  // not be in the future, so duplicate it for now.
  static struct {
    std::string operator()(double d) {
      // Going this route instead of directly using std::string to avoid
      // unnecessary decimal places.
      std::ostringstream stream;
      stream << d;
      return stream.str();
    }

    std::string operator()(std::string_view s) { return std::string(s); }
    std::string operator()(bool b) { return b ? "true" : "false"; }
    std::string operator()(std::nullptr_t) { return "nil"; }
  } visitor;
  return std::visit(visitor, value);
}

std::string_view Interpreter::saveString(std::string &&str) {
  savedStrings.push_back(std::move(str));
  return savedStrings.back();
}
