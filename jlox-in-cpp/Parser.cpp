#include "Parser.h"

#include <variant>

#include "Error.h"

std::vector<Stmt> Parser::parse() {
  std::vector<Stmt> statements;
  while (!isAtEnd())
    statements.push_back(statement());
  return statements;
}

Stmt Parser::statement() {
  if (match({TokenType::PRINT}))
    return printStatement();

  return expressionStatement();
}

Stmt Parser::printStatement() {
  Expr value = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return makeStmt<PrintStmt>(value);
}

Stmt Parser::expressionStatement() {
  Expr expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return makeStmt<ExpressionStmt>(expr);
}

Expr Parser::expression() { return equality(); }

Expr Parser::equality() {
  return binary(&Parser::comparison,
                {TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL});
}

Expr Parser::comparison() {
  return binary(&Parser::term, {TokenType::GREATER, TokenType::GREATER_EQUAL,
                                TokenType::LESS, TokenType::LESS_EQUAL});
}

Expr Parser::term() {
  return binary(&Parser::factor, {TokenType::MINUS, TokenType::PLUS});
}

Expr Parser::factor() {
  return binary(&Parser::unary, {TokenType::SLASH, TokenType::STAR});
}

Expr Parser::binary(Expr (Parser::*next)(),
                    std::initializer_list<TokenType> types) {
  Expr expr = (this->*next)();

  while (match(types)) {
    const Token &op = previous();
    Expr right = (this->*next)();
    expr = makeExpr<BinaryExpr>(expr, op, right);
  }

  return expr;
}

Expr Parser::unary() {
  if (match({TokenType::BANG, TokenType::MINUS})) {
    const Token &op = previous();
    Expr right = unary();
    return makeExpr<UnaryExpr>(op, right);
  }

  return primary();
}

Expr Parser::primary() {
  if (match({TokenType::FALSE}))
    return makeExpr<LiteralExpr>(false);
  if (match({TokenType::TRUE}))
    return makeExpr<LiteralExpr>(true);
  if (match({TokenType::NIL}))
    return makeExpr<LiteralExpr>(nullptr);

  static struct {
    Expr operator()(std::monostate) { __builtin_unreachable(); }
    Expr operator()(double d) { return p.makeExpr<LiteralExpr>(d); }
    Expr operator()(std::string_view s) { return p.makeExpr<LiteralExpr>(s); }
    Parser &p;
  } literalVisitor{*this};
  if (match({TokenType::NUMBER, TokenType::STRING}))
    return std::visit(literalVisitor, previous().literal);

  if (match({TokenType::LEFT_PAREN})) {
    Expr expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return makeExpr<GroupingExpr>(expr);
  }

  throw error(peek(), "Expect expression.");
}

bool Parser::match(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }

  return false;
}

const Token &Parser::consume(TokenType type, std::string_view message) {
  if (check(type))
    return advance();

  throw error(peek(), message);
}

bool Parser::check(TokenType type) const {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

const Token &Parser::advance() {
  if (!isAtEnd())
    ++current;
  return previous();
}

bool Parser::isAtEnd() const { return peek().type == TokenType::TOKEN_EOF; }

const Token &Parser::peek() const { return *current; }

const Token &Parser::previous() const { return *(current - 1); }

Parser::ParseError Parser::error(const Token &token, std::string_view message) {
  ::error(token, message);
  return ParseError();
}

void Parser::synchronize() {
  advance();

  while (!isAtEnd()) {
    if (previous().type == TokenType::SEMICOLON)
      return;

    switch (peek().type) {
    case TokenType::CLASS:
    case TokenType::FUN:
    case TokenType::VAR:
    case TokenType::FOR:
    case TokenType::IF:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::RETURN:
      return;
    default:
      break;
    }

    advance();
  }
}

template <class T, class... U> Expr Parser::makeExpr(U &&...args) {
  T *node = new T{std::forward<U>(args)...};
  Expr expr(node);
  ownedExprs.emplace_back(expr);
  return expr;
}

template <class T, class... U> Stmt Parser::makeStmt(U &&...args) {
  T *node = new T{std::forward<U>(args)...};
  Stmt stmt(node);
  ownedStmts.emplace_back(stmt);
  return stmt;
}

Parser::~Parser() {
  for (Expr expr : ownedExprs)
    std::visit([](auto &&ptr) { delete ptr; }, expr);
  for (Stmt stmt : ownedStmts)
    std::visit([](auto &&ptr) { delete ptr; }, stmt);
}
