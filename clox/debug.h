#pragma once

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name);
unsigned disassembleInstruction(Chunk *chunk, unsigned offset);
