#pragma once

#include <initializer_list>
#include <string_view>

#include "Expr.h"

class AstPrinter {
public:
  std::string operator()(const AssignExpr *expr);
  std::string operator()(const BinaryExpr *expr);
  std::string operator()(const CallExpr *expr);
  std::string operator()(const GetExpr *expr);
  std::string operator()(const GroupingExpr *expr);
  std::string operator()(const LiteralExpr *expr);
  std::string operator()(const LogicalExpr *expr);
  std::string operator()(const SetExpr *expr);
  std::string operator()(const SuperExpr *expr);
  std::string operator()(const ThisExpr *expr);
  std::string operator()(const UnaryExpr *expr);
  std::string operator()(const VariableExpr *expr);

private:
  std::string parenthesize(std::string_view name,
                           std::initializer_list<Expr> exprs);
};
