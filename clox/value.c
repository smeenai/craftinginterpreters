#include "value.h"

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"

void initValueArray(ValueArray *array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray *array, Value value) {
  if (array->capacity < array->count + 1) {
    unsigned oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values =
        GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
  }

  array->values[array->count] = value;
  ++array->count;
}

void freeValueArray(ValueArray *array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value) {
  switch (value.type) {
  case VAL_BOOL:
    printf(asBool(value) ? "true" : "false");
    break;
  case VAL_NIL:
    printf("nil");
    break;
  case VAL_NUMBER:
    printf("%g", asNumber(value));
    break;
  case VAL_OBJ:
    printObject(value);
    break;
  }
}

bool valuesEqual(Value a, Value b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
  case VAL_BOOL:
    return asBool(a) == asBool(b);
  case VAL_NIL:
    return true;
  case VAL_NUMBER:
    return asNumber(a) == asNumber(b);
  case VAL_OBJ: {
    ObjString *aString = asString(a);
    ObjString *bString = asString(b);
    return aString->length == bString->length &&
           memcmp(aString->chars, bString->chars, aString->length) == 0;
  }
  }
}
