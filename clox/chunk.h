#pragma once

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

typedef struct {
  unsigned count;
  unsigned capacity;
  uint8_t *code;
  unsigned *lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, unsigned line);
unsigned addConstant(Chunk *chunk, Value value);