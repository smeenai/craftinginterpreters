#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

static void repl() {
  char line[1024];
  while (true) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      puts("");
      break;
    }

    interpret(line);
  }
}

static char *readFile(const char *path) {
  FILE *file = fopen(path, "r");
  if (!file) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(EX_IOERR);
  }

  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char *buffer = malloc(fileSize + 1);
  if (!buffer) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(EX_IOERR);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(EX_IOERR);
  }

  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void runFile(const char *path) {
  char *source = readFile(path);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(EX_DATAERR);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(EX_SOFTWARE);
}

int main(int argc, const char *argv[]) {
  initVM();

  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fputs("Usage: clox[path]\n", stderr);
    exit(EX_USAGE);
  }

  freeVM();
  return 0;
}
