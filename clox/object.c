#include "object.h"

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *obj = reallocate(NULL, 0, size);
  obj->type = type;
  obj->next = vm.objects;
  vm.objects = obj;
  return obj;
}

#define ALLOCATE_OBJ(type, objType) (type *)allocateObj(sizeof(type), objType)

static ObjString *allocateString(unsigned length) {
  ObjString *string =
      (ObjString *)allocateObject(sizeof(ObjString) + length + 1, OBJ_STRING);
  string->length = length;
  return string;
}

static uint32_t hashString(const char *key, unsigned length) {
  uint32_t hash = 2166136261u;
  for (unsigned i = 0; i < length; ++i) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjString *copyString(const char *chars, unsigned length) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL)
    return interned;

  ObjString *string = allocateString(length);
  memcpy(string->chars, chars, length);
  string->chars[length] = '\0';
  string->hash = hash;
  tableSet(&vm.strings, string, nilVal());
  return string;
}

ObjString *concatenateStrings(ObjString *a, ObjString *b) {
  unsigned length = a->length + b->length;
  assert(length >= a->length && "String length overflow");
  ObjString *string = allocateString(length);
  memcpy(string->chars, a->chars, a->length);
  memcpy(string->chars + a->length, b->chars, b->length);
  string->chars[length] = '\0';
  string->hash = hashString(string->chars, length);

  ObjString *interned =
      tableFindString(&vm.strings, string->chars, length, string->hash);
  if (interned != NULL) {
    // TODO: Should we just let the garbage collector do this instead later?
    assert(vm.objects == (Obj *)string);
    vm.objects = vm.objects->next;
    reallocate(string, sizeof(ObjString) + length + 1, 0);

    return interned;
  }

  tableSet(&vm.strings, string, nilVal());
  return string;
}

void printObject(Value value) {
  switch (objType(value)) {
  case OBJ_STRING:
    printf("%s", asCString(value));
    break;
  }
}
