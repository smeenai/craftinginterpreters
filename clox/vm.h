#pragma once

#include "chunk.h"
#include "common.h"

typedef struct {
  Chunk *chunk;
  uint8_t *ip;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVM(void);
void freeVM(void);
InterpretResult interpret(Chunk *chunk);
