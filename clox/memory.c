#include "memory.h"

#include <stdlib.h>

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
