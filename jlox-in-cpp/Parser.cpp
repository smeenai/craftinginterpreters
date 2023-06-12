#include "Parser.h"

#include <variant>

#include "Error.h"

std::vector<Stmt> Parser::parse() {
  std::vector<Stmt> statements;
  while (!isAtEnd()) {
    std::optional<Stmt> decl = declaration();
    if (decl)
      statements.push_back(*decl);
  }
  return statements;
}

std::optional<Stmt> Parser::declaration() {
  try {
    if (match({TokenType::FUN}))
      return functionStatement("function");
    if (match({TokenType::VAR}))
      return varDeclaration();

    return statement();
  } catch (ParseError error) {
    synchronize();
    return {};
  }
}

Stmt Parser::varDeclaration() {
  const Token &name = consume(TokenType::IDENTIFIER, "Expect variable name.");

  std::optional<Expr> initializer;
  if (match({TokenType::EQUAL}))
    initializer = expression();

  consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
  return makeStmt<VarStmt>(name, initializer);
}

Stmt Parser::statement() {
  if (match({TokenType::FOR}))
    return forStatement();

  if (match({TokenType::IF}))
    return ifStatement();

  if (match({TokenType::PRINT}))
    return printStatement();

  if (match({TokenType::RETURN}))
    return returnStatement();

  if (match({TokenType::WHILE}))
    return whileStatement();

  if (match({TokenType::LEFT_BRACE}))
    return makeStmt<BlockStmt>(blockStatement());

  return expressionStatement();
}

Stmt Parser::forStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

  std::optional<Stmt> initializer;
  if (match({TokenType::VAR}))
    initializer = varDeclaration();
  else if (!match({TokenType::SEMICOLON}))
    initializer = expressionStatement();

  std::optional<Expr> condition;
  if (!check(TokenType::SEMICOLON))
    condition = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

  std::optional<Expr> increment;
  if (!check(TokenType::RIGHT_PAREN))
    increment = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

  Stmt body = statement();

  if (increment)
    body = makeStmt<BlockStmt>(
        std::vector<Stmt>{body, makeStmt<ExpressionStmt>(*increment)});

  if (!condition)
    condition = makeExpr<LiteralExpr>(true);
  body = makeStmt<WhileStmt>(*condition, body);

  if (initializer)
    body = makeStmt<BlockStmt>(std::vector<Stmt>{*initializer, body});

  return body;
}

Stmt Parser::ifStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  Expr condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

  Stmt thenBranch = statement();
  std::optional<Stmt> elseBranch;
  if (match({TokenType::ELSE}))
    elseBranch = statement();

  return makeStmt<IfStmt>(condition, thenBranch, elseBranch);
}

Stmt Parser::printStatement() {
  Expr value = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return makeStmt<PrintStmt>(value);
}

Stmt Parser::returnStatement() {
  const Token &keyword = previous();
  std::optional<Expr> value;
  if (!check(TokenType::SEMICOLON))
    value = expression();

  consume(TokenType::SEMICOLON, "Expect ';' after return value.");
  return makeStmt<ReturnStmt>(keyword, value);
}

Stmt Parser::whileStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  Expr condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
  Stmt body = statement();
  return makeStmt<WhileStmt>(condition, body);
}

Stmt Parser::expressionStatement() {
  Expr expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return makeStmt<ExpressionStmt>(expr);
}

Stmt Parser::functionStatement(const std::string &kind) {
  const Token &name =
      consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
  consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
  std::vector<std::reference_wrapper<const Token>> parameters;
  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      parameters.emplace_back(
          consume(TokenType::IDENTIFIER, "Expect parameter name."));
    } while (match({TokenType::COMMA}));
  }

  if (parameters.size() > 255)
    error(peek(), "Can't have more than 255 parameters.");

  consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

  consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
  std::vector<Stmt> body = blockStatement();
  return makeStmt<FunctionStmt>(name, std::move(parameters), std::move(body));
}

std::vector<Stmt> Parser::blockStatement() {
  std::vector<Stmt> statements;

  while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    if (std::optional<Stmt> maybeStmt = declaration())
      statements.push_back(*maybeStmt);

  consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
  return statements;
}

Expr Parser::expression() { return assignment(); }

Expr Parser::assignment() {
  Expr expr = orExpression();

  if (match({TokenType::EQUAL})) {
    const Token &equals = previous();
    Expr value = assignment();

    if (const VariableExpr **variableExpr =
            std::get_if<const VariableExpr *>(&expr))
      return makeExpr<AssignExpr>((*variableExpr)->name, value);

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

Expr Parser::orExpression() {
  return binary<LogicalExpr>(&Parser::andExpression, {TokenType::OR});
}

Expr Parser::andExpression() {
  return binary<LogicalExpr>(&Parser::equality, {TokenType::AND});
}

Expr Parser::equality() {
  return binary<BinaryExpr>(&Parser::comparison,
                            {TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL});
}

Expr Parser::comparison() {
  return binary<BinaryExpr>(&Parser::term,
                            {TokenType::GREATER, TokenType::GREATER_EQUAL,
                             TokenType::LESS, TokenType::LESS_EQUAL});
}

Expr Parser::term() {
  return binary<BinaryExpr>(&Parser::factor,
                            {TokenType::MINUS, TokenType::PLUS});
}

Expr Parser::factor() {
  return binary<BinaryExpr>(&Parser::unary,
                            {TokenType::SLASH, TokenType::STAR});
}

template <class T>
Expr Parser::binary(Expr (Parser::*next)(),
                    std::initializer_list<TokenType> types) {
  Expr expr = (this->*next)();

  while (match(types)) {
    const Token &op = previous();
    Expr right = (this->*next)();
    expr = makeExpr<T>(expr, op, right);
  }

  return expr;
}

Expr Parser::unary() {
  if (match({TokenType::BANG, TokenType::MINUS})) {
    const Token &op = previous();
    Expr right = unary();
    return makeExpr<UnaryExpr>(op, right);
  }

  return call();
}

Expr Parser::call() {
  Expr expr = primary();

  while (true) {
    if (match({TokenType::LEFT_PAREN}))
      expr = finishCall(expr);
    else
      break;
  }

  return expr;
}

Expr Parser::finishCall(Expr callee) {
  std::vector<Expr> arguments;
  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      arguments.push_back(expression());
    } while (match({TokenType::COMMA}));
  }

  if (arguments.size() > 255)
    error(peek(), "Can't have more than 255 arguments.");

  const Token &paren =
      consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
  return makeExpr<CallExpr>(callee, paren, std::move(arguments));
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

  if (match({TokenType::IDENTIFIER}))
    return makeExpr<VariableExpr>(previous());

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
