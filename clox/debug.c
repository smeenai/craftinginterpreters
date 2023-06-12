#include "debug.h"

#include <stdio.h>

#include "value.h"

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== %s ==\n", name);
  for (unsigned offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

static unsigned constantInstruction(const char *name, Chunk *chunk,
                                    unsigned offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4u '", name, constant);
  printValue(chunk->constants.values[constant]);
  puts("'");
  return offset + 2;
}

static unsigned simpleInstruction(const char *name, unsigned offset) {
  puts(name);
  return offset + 1;
}

unsigned disassembleInstruction(Chunk *chunk, unsigned offset) {
  printf("%04u ", offset);
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    printf("   | ");
  else
    printf("%4u ", chunk->lines[offset]);

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset);

  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);

  default:
    printf("Unknown opcode %u\n", instruction);
    return offset + 1;
  }
}
