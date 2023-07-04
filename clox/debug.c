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

  case OP_NIL:
    return simpleInstruction("OP_NIL", offset);

  case OP_TRUE:
    return simpleInstruction("OP_TRUE", offset);

  case OP_FALSE:
    return simpleInstruction("OP_FALSE", offset);

  case OP_ADD:
    return simpleInstruction("OP_ADD", offset);

  case OP_SUBTRACT:
    return simpleInstruction("OP_SUBTRACT", offset);

  case OP_MULTIPLY:
    return simpleInstruction("OP_MULTIPLY", offset);

  case OP_DIVIDE:
    return simpleInstruction("OP_DIVIDE", offset);

  case OP_NOT:
    return simpleInstruction("OP_NOT", offset);

  case OP_NEGATE:
    return simpleInstruction("OP_NEGATE", offset);

  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);

  default:
    printf("Unknown opcode %u\n", instruction);
    return offset + 1;
  }
}
