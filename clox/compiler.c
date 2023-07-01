#include "compiler.h"

#include <stdio.h>

#include "common.h"
#include "scanner.h"

void compile(const char *source) {
  initScanner(source);
  unsigned line = 0;
  while (true) {
    Token token = scanToken();
    if (token.line != line) {
      printf("%4d ", token.line);
      line = token.line;
    } else {
      printf("   | ");
    }
    printf("%-13s '%.*s'\n", getTokenTypeName(token.type), (int)token.length,
           token.start);

    if (token.type == TOKEN_EOF)
      break;
  }
}
