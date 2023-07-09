#include "memory.h"

#include <stdlib.h>

#include "object.h"
#include "vm.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  (void)oldSize; // Why do we even have that lever?

  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  void *result = realloc(pointer, newSize);
  if (result == NULL)
    exit(1);

  return result;
}

static void freeObject(Obj *obj) {
  switch (obj->type) {
  case OBJ_STRING:
    reallocate(obj, sizeof(ObjString) + ((ObjString *)obj)->length + 1, 0);
    break;
  }
}

void freeObjects() {
  Obj *obj = vm.objects;
  while (obj != NULL) {
    Obj *next = obj->next;
    freeObject(obj);
    obj = next;
  }
}
