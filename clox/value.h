#pragma once

#include <assert.h>

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj;
  } as;
} Value;

// I'm using inline functions instead of macros because we may as well. I'm
// marking them always_inline because -O0 wouldn't inline otherwise. C99 inline
// semantics (as opposed to the C++ or gnu89 ones) are that a definition is
// never emitted for an inline function, and I'm not a providing a non-inline
// definition, so any failed inlinings will be a linker error. Yes, I'm aware of
// the irony of using a macro for the attribute.
#define ALWAYS_INLINE __attribute__((__always_inline__)) inline

// Taking values by pointer might be better, but that complicates other places,
// and post-inlining optimizations should turn this into a direct field access:
// https://godbolt.org/z/WKcb17hGc
ALWAYS_INLINE bool isBool(Value value) { return value.type == VAL_BOOL; }
ALWAYS_INLINE bool isNil(Value value) { return value.type == VAL_NIL; }
ALWAYS_INLINE bool isNumber(Value value) { return value.type == VAL_NUMBER; }
ALWAYS_INLINE bool isObj(Value value) { return value.type == VAL_OBJ; }

ALWAYS_INLINE bool asBool(Value value) {
  assert(isBool(value) && "Called asBool on non-bool");
  return value.as.boolean;
}
ALWAYS_INLINE double asNumber(Value value) {
  assert(isNumber(value) && "Called asNumber on non-number");
  return value.as.number;
}
ALWAYS_INLINE Obj *asObj(Value value) {
  assert(isObj(value) && "Called asObj on non-Obj");
  return value.as.obj;
}

ALWAYS_INLINE Value boolVal(bool value) {
  return (Value){VAL_BOOL, {.boolean = value}};
}
ALWAYS_INLINE Value nilVal() { return (Value){VAL_NIL, {.number = 0}}; }
ALWAYS_INLINE Value numberVal(double value) {
  return (Value){VAL_NUMBER, {.number = value}};
}
ALWAYS_INLINE Value objVal(Obj *obj) { return (Value){VAL_OBJ, {.obj = obj}}; }

#undef ALWAYS_INLINE

#define OBJ_VAL(obj) _Generic((obj), ObjString *: objVal((Obj *)obj))

typedef struct {
  unsigned capacity;
  unsigned count;
  Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);
