#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char *argv[]) {
  (void)argc;
  (void)argv;

  initVM();

  Chunk chunk;
  initChunk(&chunk);

  unsigned constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_RETURN, 123);

  disassembleChunk(&chunk, "test chunk");
  interpret(&chunk);
  freeChunk(&chunk);

  freeVM();

  return 0;
}
