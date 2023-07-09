#pragma once

#include "common.h"
#include "value.h"

typedef enum {
  OBJ_STRING,
} ObjType;

typedef struct Obj {
  ObjType type;
  struct Obj *next;
} Obj;

struct ObjString {
  Obj obj;
  unsigned length;
  char chars[];
};

ObjString *copyString(const char *chars, unsigned length);
ObjString *concatenateStrings(ObjString *a, ObjString *b);

void printObject(Value value);

// See value.h for an explanation.
#define ALWAYS_INLINE __attribute__((__always_inline__)) inline

ALWAYS_INLINE ObjType objType(Value value) { return asObj(value)->type; }

ALWAYS_INLINE bool isObjType(Value value, ObjType type) {
  return isObj(value) && objType(value) == type;
}

ALWAYS_INLINE bool isString(Value value) {
  return isObjType(value, OBJ_STRING);
}

ALWAYS_INLINE ObjString *asString(Value value) {
  assert(isString(value) && "Called asString on non-string");
  return (ObjString *)asObj(value);
}

ALWAYS_INLINE char *asCString(Value value) {
  assert(isString(value) && "Called asCString on non-string");
  return ((ObjString *)asObj(value))->chars;
}

#undef ALWAYS_INLINE
