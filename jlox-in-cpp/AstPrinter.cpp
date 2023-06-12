#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>

#include "Expr.h"
#include "Token.h"

class AstPrinter {
public:
  std::string operator()(const BinaryExpr *expr) {
    return parenthesize(expr->op.lexeme, {expr->left, expr->right});
  }

  std::string operator()(const GroupingExpr *expr) {
    return parenthesize("group", {expr->expr});
  }

  std::string operator()(const LiteralExpr *expr) {
    static struct {
      std::string operator()(double d) { return std::to_string(d); }
      std::string operator()(std::string_view s) { return std::string(s); }
      std::string operator()(bool b) { return b ? "true" : "false"; }
      std::string operator()(std::nullptr_t) { return "nil"; }
    } visitor;
    return std::visit(visitor, expr->value);
  }

  std::string operator()(const UnaryExpr *expr) {
    return parenthesize(expr->op.lexeme, {expr->right});
  }

private:
  std::string parenthesize(std::string_view name,
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
};

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
