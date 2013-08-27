#ifndef __STD_HASHMAP_H__
#define __STD_HASHMAP_H__

#include "std/types.h"

typedef struct HashMap HashMapT;

HashMapT *NewHashMap(size_t initialSize);

/**
 * @begin Makes internal copy of the key. Takes ownership of the value.
 */
void HashMapAdd(HashMapT *self, const char *key, PtrT value);

/**
 * @begin Makes internal copy of the key. Doesn't take ownership of the value.
 */
void HashMapAddLink(HashMapT *self, const char *key, PtrT value);

PtrT HashMapFind(HashMapT *self, const char *key);
PtrT HashMapRemove(HashMapT *self, const char *key);

typedef void (*HashMapIterFuncT)(const char *key, PtrT value);

void HashMapIter(HashMapT *self, HashMapIterFuncT func);

#endif
