#include "vm.h"

#include <stdio.h>

#include "debug.h"

static VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) { *vm.stackTop++ = value; }

Value pop() { return *--vm.stackTop; }

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double b = pop();                                                          \
    double a = pop();                                                          \
    push(a op b);                                                              \
  } while (false)

  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = vm.stack; slot < vm.stackTop; ++slot) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    putchar('\n');
    disassembleInstruction(vm.chunk, (unsigned)(vm.ip - vm.chunk->code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }

    case OP_ADD:
      BINARY_OP(+);
      break;

    case OP_SUBTRACT:
      BINARY_OP(-);
      break;

    case OP_MULTIPLY:
      BINARY_OP(*);
      break;

    case OP_DIVIDE:
      BINARY_OP(/);
      break;

    case OP_NEGATE:
      *(vm.stackTop - 1) = -*(vm.stackTop - 1);
      break;

    case OP_RETURN:
      printValue(pop());
      putchar('\n');
      return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk *chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}
