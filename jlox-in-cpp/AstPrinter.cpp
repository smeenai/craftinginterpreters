#include "AstPrinter.h"

#include <iostream>
#include <sstream>
#include <string>
#include <variant>

#include "Expr.h"
#include "Token.h"

std::string AstPrinter::operator()(const AssignExpr *expr) {
  return std::string(expr->name.lexeme) + " = " +
         std::visit(*this, expr->value);
}

std::string AstPrinter::operator()(const BinaryExpr *expr) {
  return parenthesize(expr->op.lexeme, {expr->left, expr->right});
}

std::string AstPrinter::operator()(const CallExpr *expr) {
  std::string str = std::visit(*this, expr->callee) + "(";
  for (const Expr subExpr : expr->arguments)
    str += std::visit(*this, subExpr);
  return str;
}

std::string AstPrinter::operator()(const GetExpr *expr) {
  return std::visit(*this, expr->object) + "." + std::string(expr->name.lexeme);
}

std::string AstPrinter::operator()(const GroupingExpr *expr) {
  return parenthesize("group", {expr->expr});
}

std::string AstPrinter::operator()(const LiteralExpr *expr) {
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
  return std::visit(visitor, expr->value);
}

std::string AstPrinter::operator()(const LogicalExpr *expr) {
  return parenthesize(expr->op.lexeme, {expr->left, expr->right});
}

std::string AstPrinter::operator()(const SetExpr *expr) {
  return std::visit(*this, expr->object) + "." +
         std::string(expr->name.lexeme) + " = " +
         std::visit(*this, expr->value);
}

std::string AstPrinter::operator()(const ThisExpr *) { return "this"; }

std::string AstPrinter::operator()(const UnaryExpr *expr) {
  return parenthesize(expr->op.lexeme, {expr->right});
}

std::string AstPrinter::operator()(const VariableExpr *expr) {
  return std::string(expr->name.lexeme);
}

std::string AstPrinter::parenthesize(std::string_view name,
                                     std::initializer_list<Expr> exprs) {
  std::string str("(");
  str.append(name);
  for (Expr expr : exprs) {
    str.append(" ");
    str.append(std::visit(*this, expr));
  }
  str.append(")");
  return str;
}

#if defined(AST_PRINTER_STANDALONE)
int main() {
  // Can't construct from temporaries because of lifetime issues
  LiteralExpr oneTwoThree{123.};
  Token minus(TokenType::MINUS, "-", 1);
  UnaryExpr unary{minus, &oneTwoThree};

  Token star(TokenType::STAR, "*", 1);
  LiteralExpr fourFiveSixSeven{45.67};
  GroupingExpr grouping{&fourFiveSixSeven};
  BinaryExpr binary{&unary, star, &grouping};

  std::cout << std::visit(AstPrinter(), Expr(&binary)) << "\n";
}
#endif
