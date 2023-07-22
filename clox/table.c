#include "table.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

static Entry *findEntry(Entry *entries, unsigned capacity, ObjString *key) {
  uint32_t index = key->hash % capacity;
  Entry *tombstone = NULL;

  while (true) {
    Entry *entry = &entries[index];
    if (entry->key == NULL) {
      if (isNil(entry->value)) {
        // Empty entry.
        return tombstone != NULL ? tombstone : entry;
      } else {
        // We found a tombstone.
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (entry->key == key) {
      // We found the key.
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

static void adjustCapacity(Table *table, unsigned capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);
  for (unsigned i = 0; i < capacity; ++i) {
    entries[i].key = NULL;
    entries[i].value = nilVal();
  }

  table->count = 0;
  for (unsigned i = 0; i < table->capacity; ++i) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL)
      continue;

    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    ++table->count;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool tableGet(Table *table, ObjString *key, Value *value) {
  if (table->count == 0)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

bool tableSet(Table *table, ObjString *key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    unsigned capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey && isNil(entry->value))
    ++table->count;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableDelete(Table *table, ObjString *key) {
  if (table->count == 0)
    return false;

  // Find the entry.
  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  // Place a tombstone in the entry.
  entry->key = NULL;
  entry->value = boolVal(true);
  return true;
}

void tableAddAll(Table *from, Table *to) {
  for (unsigned i = 0; i < from->capacity; ++i) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL)
      tableSet(to, entry->key, entry->value);
  }
}
