#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "Expr.h"
#include "Stmt.h"

class Interpreter {
public:
  // This is equivalent to the LiteralExpr variant right now, but I suspect it
  // won't be after subsequent chapters, so I'm keeping it separate.
  using Value = std::variant<std::nullptr_t, bool, double, std::string_view>;

  void interpret(const std::vector<Stmt> &statements);

  void operator()(const ExpressionStmt *stmt);
  void operator()(const PrintStmt *stmt);

  Value operator()(const LiteralExpr *expr);
  Value operator()(const GroupingExpr *expr);
  Value operator()(const UnaryExpr *expr);
  Value operator()(const BinaryExpr *expr);

private:
  static bool isTruthy(Value value);
  static void checkNumberOperand(const Token &token, Value value);
  static void checkNumberOperands(const Token &token, Value left, Value right);

  static std::string stringify(Value value);

  // This is inspired by LLVM's StringSaver. I'm not sure if it'd be better to
  // just make Value hold std::string instead of std::string_view instead, but
  // then Value would be much larger, require a destructor, etc. This project
  // is really making me appreciate how much mental overhead a GC reduces :)
  std::string_view saveString(std::string &&str);
  std::vector<std::string> savedStrings;
};
