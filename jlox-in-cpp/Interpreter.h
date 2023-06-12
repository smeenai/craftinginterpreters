#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "Expr.h"
#include "Stmt.h"
#include "Value.h"

class Interpreter {
public:
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

  // This is inspired by LLVM's StringSaver. I'm not sure if it'd be better to
  // just make Value hold std::string instead of std::string_view instead, but
  // then Value would be much larger, require a destructor, etc. This project
  // is really making me appreciate how much mental overhead a GC reduces :)
  std::string_view saveString(std::string &&str);
  std::vector<std::string> savedStrings;
};
