#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <sysexits.h>
#include <utility>
#include <variant>
#include <vector>

#include "AstPrinter.h"
#include "Error.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Resolver.h"
#include "Scanner.h"

static Interpreter interpreter;

static void run(std::string_view source) {
  Scanner scanner(source);
  const std::vector<Token> &tokens = scanner.scanTokens();
  Parser parser(tokens);
  std::vector<Stmt> statements = parser.parse();

  // Stop if there was a syntax error.
  if (hadError())
    return;

  Resolver resolver(interpreter);
  resolver.resolve(statements);

  // Stop if there was a resolution error.
  if (hadError())
    return;

  interpreter.interpret(statements);
}

static void runPrompt() {
  // Keep all input lines alive to match file behavior and simplify everything.
  // We need the string objects to not move around since we have string_views
  // referencing them, so we use a deque instead of a vector. Note that the
  // small string optimization means that the string objects themselves need to
  // stay fixed in place and not just their heap allocations.
  std::deque<std::string> inputLines;
  while (true) {
    std::string &inputLine = inputLines.emplace_back();
    std::getline(std::cin, inputLine);
    if (inputLine.empty())
      break;

    run(inputLine);
    clearError();
  }
}

static int runFile(const char *path) {
  std::ifstream fileStream(path);
  std::stringstream source;
  source << fileStream.rdbuf();
#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 170000
  // Assuming https://reviews.llvm.org/D148641 gets merged in time.
  run(source.view());
#else
  // Create an unnecessary copy until libc++ adds stringstream::view, sigh
  run(source.str());
#endif
  return hadError() ? EX_DATAERR : hadRuntimeError() ? EX_SOFTWARE : 0;
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cout << "Usage: jlox-cpp [script]\n";
    return EX_USAGE;
  } else if (argc == 2) {
    return runFile(argv[1]);
  } else {
    runPrompt();
  }
}
