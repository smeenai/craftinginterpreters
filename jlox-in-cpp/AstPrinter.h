#pragma once

#include <initializer_list>
#include <string_view>

#include "Expr.h"

class AstPrinter {
public:
  std::string operator()(const AssignExpr *expr);
  std::string operator()(const BinaryExpr *expr);
  std::string operator()(const GroupingExpr *expr);
  std::string operator()(const LiteralExpr *expr);
  std::string operator()(const UnaryExpr *expr);
  std::string operator()(const VariableExpr *expr);

private:
  std::string parenthesize(std::string_view name,
                           std::initializer_list<Expr> exprs);
};
