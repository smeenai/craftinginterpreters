#include "vm.h"

#include <stdarg.h>
#include <stdio.h>

#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

__attribute__((format(printf, 1, 2))) static void
runtimeError(const char *format, ...) {
  va_list(args);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  unsigned line = vm.chunk->lines[instruction];
  fprintf(stderr, "\n[line %d] in script\n", line);
  resetStack();
}

void initVM() {
  resetStack();
  vm.objects = NULL;
  initTable(&vm.globals);
  initTable(&vm.strings);
}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
}

void push(Value value) { *vm.stackTop++ = value; }

Value pop() { return *--vm.stackTop; }

static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static bool isFalsey(Value value) {
  return isNil(value) || (isBool(value) && !asBool(value));
}

static void concatenate() {
  ObjString *b = asString(pop());
  ObjString *a = asString(pop());
  ObjString *result = concatenateStrings(a, b);
  push(OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_STRING() asString(READ_CONSTANT())

#define BINARY_OP_WITH_ERROR(valueType, op, typeErrorMessage)                  \
  do {                                                                         \
    if (!isNumber(peek(0)) || !isNumber(peek(1))) {                            \
      runtimeError(typeErrorMessage);                                          \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = asNumber(pop());                                                \
    double a = asNumber(pop());                                                \
    push(valueType(a op b));                                                   \
  } while (false)

#define BINARY_OP(valueType, op)                                               \
  BINARY_OP_WITH_ERROR(valueType, op, "Operands must be numbers.")

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
    case OP_POP:
      pop();
      break;

    case OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      push(vm.stack[slot]);
      break;
    }

    case OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      vm.stack[slot] = peek(0);
      break;
    }

    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }

    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING();
      tableSet(&vm.globals, name, peek(0));
      pop();
      break;
    }

    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING();
      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }

    case OP_EQUAL: {
      Value a = pop();
      Value b = pop();
      push(boolVal(valuesEqual(a, b)));
      break;
    }

    case OP_GREATER:
      BINARY_OP(boolVal, >);
      break;

    case OP_LESS:
      BINARY_OP(boolVal, <);
      break;

    case OP_ADD:
      if (isString(peek(0)) && isString(peek(1)))
        concatenate();
      else
        BINARY_OP_WITH_ERROR(numberVal, +,
                             "Operands must be two numbers or two strings.");
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

    case OP_PRINT: {
      printValue(pop());
      putchar('\n');
      break;
    }

    case OP_RETURN:
      // Exit interpreter.
      return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
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
