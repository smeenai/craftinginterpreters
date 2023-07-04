#include "vm.h"

#include <stdarg.h>
#include <stdio.h>

#include "compiler.h"
#include "debug.h"

static VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

static void runtimeError(const char *format, ...) {
  va_list(args);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  unsigned line = vm.chunk->lines[instruction];
  fprintf(stderr, "\n[line %d] in script\n", line);
  resetStack();
}

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) { *vm.stackTop++ = value; }

Value pop() { return *--vm.stackTop; }

static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static bool isFalsey(Value value) {
  return isNil(value) || (isBool(value) && !asBool(value));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!isNumber(peek(0)) || !isNumber(peek(1))) {                            \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = asNumber(pop());                                                \
    double a = asNumber(pop());                                                \
    push(valueType(a op b));                                                   \
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

    case OP_NIL:
      push(nilVal());
      break;
    case OP_TRUE:
      push(boolVal(true));
      break;
    case OP_FALSE:
      push(boolVal(false));
      break;

    case OP_ADD:
      BINARY_OP(numberVal, +);
      break;

    case OP_SUBTRACT:
      BINARY_OP(numberVal, -);
      break;

    case OP_MULTIPLY:
      BINARY_OP(numberVal, *);
      break;

    case OP_DIVIDE:
      BINARY_OP(numberVal, /);
      break;

    case OP_NOT:
      push(boolVal(isFalsey(pop())));
      break;

    case OP_NEGATE:
      if (!isNumber(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(numberVal(-asNumber(pop())));
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

InterpretResult interpret(const char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}
