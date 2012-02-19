#ifndef __STD_ATOMPOOL_H__
#define __STD_ATOMPOOL_H__

#include "std/types.h"

typedef struct AtomPool {
  void *pool;
  size_t atomSize;
  size_t perChunk;
} AtomPoolT;

AtomPoolT *NewAtomPool(size_t atomSize, size_t perChunk);
void DeleteAtomPool(AtomPoolT *atomPool);
void *AtomNew(AtomPoolT *atomPool);
void *AtomNew0(AtomPoolT *atomPool);
void AtomFree(AtomPoolT *atomPool, void *atom);

#endif
