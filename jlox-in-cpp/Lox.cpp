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
#include "Parser.h"
#include "Scanner.h"

static void run(std::string_view source) {
  Scanner scanner(source);
  const std::vector<Token> &tokens = scanner.scanTokens();
  Parser parser(tokens);
  std::optional<Expr> expr = parser.parse();

  // Stop if there was a syntax error.
  if (hadError())
    return;

  std::cout << std::visit(AstPrinter(), expr.value()) << "\n";
}

static void runPrompt() {
  std::string inputLine;
  while (true) {
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
#if __cplusplus >= 202002L
  run(source.view());
#else
  // libc++ doesn't have stringstream::view yet, so we create an unnecessary
  // copy here, sigh. https://reviews.llvm.org/D148641
  run(source.str());
#endif
  return hadError() ? EX_DATAERR : 0;
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
