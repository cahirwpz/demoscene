#ifndef __STD_HASHMAP_H__
#define __STD_HASHMAP_H__

#include "std/types.h"

typedef struct HashMap HashMapT;

HashMapT *NewHashMap(size_t initialSize);

/**
 * @begin Makes internal copy of the key. Takes ownership of the value.
 */
void HashMapAdd(HashMapT *self, StrT key, PtrT value);

/**
 * @begin Makes internal copy of the key. Doesn't take ownership of the value.
 */
void HashMapAddLink(HashMapT *self, StrT key, PtrT value);

PtrT HashMapFind(HashMapT *self, StrT key);
PtrT HashMapRemove(HashMapT *self, StrT key);

typedef void (*HashMapIterFuncT)(const StrT key, PtrT value);

void HashMapIter(HashMapT *self, HashMapIterFuncT func);

#endif
