#pragma once

#include <initializer_list>
#include <optional>
#include <string_view>
#include <vector>

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"

class Parser {
public:
  Parser(const std::vector<Token> &tokens) : current(tokens.begin()) {}
  std::vector<Stmt> parse();

  ~Parser();
  // These shouldn't be needed, and accidentally attempting to use them would
  // cause memory errors, so give a loud error instead.
  Parser(const Parser &) = delete;
  Parser(Parser &&) = delete;
  Parser &operator=(const Parser &) = delete;
  Parser &operator=(Parser &&) = delete;

private:
  std::vector<Token>::const_iterator current;

  std::optional<Stmt> declaration();
  Stmt varDeclaration();
  Stmt statement();
  Stmt ifStatement();
  Stmt printStatement();
  Stmt expressionStatement();
  std::vector<Stmt> blockStatement();

  Expr expression();
  Expr assignment();

  Expr equality();
  Expr comparison();
  Expr term();
  Expr factor();
  Expr binary(Expr (Parser::*next)(), std::initializer_list<TokenType> types);

  Expr unary();
  Expr primary();

  bool match(std::initializer_list<TokenType> types);
  const Token &consume(TokenType type, std::string_view message);
  bool check(TokenType type) const;
  const Token &advance();
  bool isAtEnd() const;
  const Token &peek() const;
  const Token &previous() const;

  // We would normally inherit from a standard exception type, but this seems to
  // purely be a sentinel, so that seems overkill.
  class ParseError {};
  ParseError error(const Token &token, std::string_view message);
  void synchronize();

  // This is a simple memory management scheme to have the parser allocate and
  // free the memory for each AST node, rather than the nodes needing to take
  // any ownership themselves (which would make it hard to e.g. stack-allocate
  // nodes).
  template <class T, class... U> Expr makeExpr(U &&...args);
  std::vector<Expr> ownedExprs;

  template <class T, class... U> Stmt makeStmt(U &&...args);
  std::vector<Stmt> ownedStmts;
};
