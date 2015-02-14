#include "std/debug.h"
#include "std/memory.h"
#include "std/hashmap.h"

typedef struct Entry {
  struct Entry *next;
  char *key;
  PtrT value;
  bool ownership;
} EntryT;

struct HashMap {
  EntryT *map;
};

static const size_t HashTableSize[] = { 89, 179, 359, 719, 1439, 2879, 5759,
                                        11519, 23039, 46079, 92159, 184319,
                                        368639, 737279, 1474559 };

static void DeleteHashMap(HashMapT *self) {
  size_t n = TableSize(self->map);
  size_t i;

  for (i = 0; i < n; i++) {
    EntryT *entry = &self->map[i];
    bool firstEntry = true;

    if (!entry->key)
      continue;

    while (entry) {
      EntryT *next = entry->next;
      MemUnref(entry->key);
      if (entry->ownership)
        MemUnref(entry->value);
      if (!firstEntry)
        MemUnref(entry);

      firstEntry = false;
      entry = next;
    }
  }

  MemUnref(self->map);
}

TYPEDECL(HashMapT, (FreeFuncT)DeleteHashMap);

HashMapT *NewHashMap(size_t initialSize) {
  HashMapT *self = NewInstance(HashMapT);
  
  {
    size_t n = 0;
    size_t i;

    for (i = 0; i < sizeof(HashTableSize) / sizeof(size_t); i++) {
      if (HashTableSize[i] > initialSize) {
        n = HashTableSize[i];
        break;
      }
    }

    if (n == 0)
      PANIC("Table too large (%ld)", initialSize);

    self->map = NewTable(EntryT, n);
  }

  return self;
}

static EntryT *GetEntry(HashMapT *self, const char *str) {
  size_t hash = 0;
  size_t c;

  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  return &self->map[hash % TableSize(self->map)];
}

/* TODO: Update value if key exists. */
static void AddEntry(HashMapT *self, const char *key, PtrT value, bool ownership) {
  EntryT *entry = GetEntry(self, key);

  if (entry->key) {
    EntryT *next = NewRecord(EntryT);

    *next = *entry;

    entry->next = next;
  } else {
    entry->next = NULL;
  }

  entry->key = StrDup(key);
  entry->value = value;
  entry->ownership = ownership;
}

void HashMapAdd(HashMapT *self, const char *key, PtrT value) {
  AddEntry(self, key, value, true);
}

void HashMapAddLink(HashMapT *self, const char *key, PtrT value) {
  AddEntry(self, key, value, false);
}

static EntryT *FindEntry(HashMapT *self, const char *key, EntryT **prev) {
  EntryT *entry = GetEntry(self, key);

  if (!entry->key)
    entry = NULL;

  *prev = NULL;

  while (entry && (strcmp(entry->key, key) != 0)) {
    *prev = entry;
    entry = entry->next;
  }

  return entry;
}

PtrT HashMapFind(HashMapT *self, const char *key) {
  EntryT *prev;
  EntryT *entry = FindEntry(self, key, &prev);

  return entry ? entry->value : NULL;
}

PtrT HashMapRemove(HashMapT *self, const char *key) {
  EntryT *prev;
  EntryT *entry = FindEntry(self, key, &prev);
  PtrT value = NULL;

  if (entry) {
    value = entry->value;

    MemUnref(entry->key);

    if (!prev) {
      entry->next  = NULL;
      entry->key   = NULL;
      entry->value = NULL;
    } else {
      prev->next = entry->next;
      MemUnref(entry);
    }
  }

  return value;
}

void HashMapIter(HashMapT *self, HashMapIterFuncT func) {
  size_t n = TableSize(self->map);
  size_t i;

  for (i = 0; i < n; i++) {
    EntryT *entry = &self->map[i];

    if (!entry->key)
      continue;

    while (entry) {
      func(entry->key, entry->value);
      entry = entry->next;
    }
  }
}
